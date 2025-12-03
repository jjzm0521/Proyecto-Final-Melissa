/*
 * rc522.c
 * Basado en la librería MFRC522 provista.
 */

#include "rc522.h"
// include "spi.h"  // Necesario para hspi1

// --- CONFIGURACIÓN DE PINES (AJUSTAR AQUI SI CAMBIAS EL HARDWARE) ---
// NSS (Chip Select) -> PA4 (Changed from PC1 to avoid conflict with Keypad)
#define RC522_CS_PORT GPIOA
#define RC522_CS_PIN GPIO_PIN_4

// RESET -> PB0
#define RC522_RST_PORT GPIOB
#define RC522_RST_PIN GPIO_PIN_0

// --- Funciones de Bajo Nivel (SPI) ---

void MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
  // Formato: Dirección desplazada + 0 en MSB (Write)
  uint8_t addr_bits = (addr << 1) & 0x7E;

  HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_RESET); // Select
  HAL_SPI_Transmit(&hspi1, &addr_bits, 1, 500);
  HAL_SPI_Transmit(&hspi1, &val, 1, 500);
  HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_SET); // Deselect
}

uint8_t MFRC522_ReadRegister(uint8_t addr) {
  uint8_t rx_bits;
  // Formato: Dirección desplazada + 1 en MSB (Read)
  uint8_t addr_bits = ((addr << 1) & 0x7E) | 0x80;

  HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_RESET); // Select
  HAL_SPI_Transmit(&hspi1, &addr_bits, 1, 500);
  HAL_SPI_Receive(&hspi1, &rx_bits, 1, 500);
  HAL_GPIO_WritePin(RC522_CS_PORT, RC522_CS_PIN, GPIO_PIN_SET); // Deselect

  return rx_bits;
}

void MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
  uint8_t tmp;
  tmp = MFRC522_ReadRegister(reg);
  MFRC522_WriteRegister(reg, tmp | mask); // set bit mask
}

void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask) {
  uint8_t tmp;
  tmp = MFRC522_ReadRegister(reg);
  MFRC522_WriteRegister(reg, tmp & (~mask)); // clear bit mask
}

// --- Funciones de Control del Módulo ---

void MFRC522_Reset(void) {
  // Hard Reset
  // RST pin: High = Reset/PowerDown, Low = Normal Operation
  HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN, GPIO_PIN_SET); // Reset
  HAL_Delay(10);
  HAL_GPIO_WritePin(RC522_RST_PORT, RC522_RST_PIN,
                    GPIO_PIN_RESET); // Release Reset (Work)
  HAL_Delay(50);

  // Soft Reset
  MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
  HAL_Delay(50);
}

void MFRC522_AntennaOn(void) {
  uint8_t temp;
  temp = MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
  if (!(temp & 0x03)) {
    MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
  }
}

void MFRC522_AntennaOff(void) {
  MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

void MFRC522_Init(void) {
  MFRC522_Reset();

  // Configuración del Timer (Según librería original)
  // TPrescaler*TreloadVal/13.56MHz = tiempo espera
  MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x80);      // Tauto=1
  MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0xA9); // Prescaler
  MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0x03);  // Reload High
  MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 0xE8);  // Reload Low

  MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40); // 100% ASK
  MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);    // CRC 0x6363

  // Aumentar ganancia de antena (Opcional, pero recomendado para clones)
  MFRC522_WriteRegister(MFRC522_REG_RF_CFG, 0x70); // 48dB Gain

  MFRC522_AntennaOn();
}

uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen,
                       uint8_t *backData, uint16_t *backLen) {
  uint8_t status = MI_ERR;
  uint8_t irqEn = 0x00;
  uint8_t waitIRq = 0x00;
  uint8_t lastBits;
  uint8_t n;
  uint16_t i;

  switch (command) {
  case PCD_AUTHENT:
    irqEn = 0x12;
    waitIRq = 0x10;
    break;
  case PCD_TRANSCEIVE:
    irqEn = 0x77;
    waitIRq = 0x30;
    break;
  default:
    break;
  }

  MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
  MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);
  MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80); // FlushBuffer

  MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);

  // Escribir datos
  for (i = 0; i < sendLen; i++) {
    MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);
  }

  // Ejecutar comando
  MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
  if (command == PCD_TRANSCEIVE) {
    MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80); // StartSend
  }

  // Esperar a que termine
  i = 25000;
  do {
    n = MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
    i--;
  } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

  MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80); // StopSend

  if (i != 0) {
    if (!(MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x1B)) {
      status = MI_OK;
      if (n & irqEn & 0x01) {
        status = MI_NOTAGERR;
      }

      if (command == PCD_TRANSCEIVE) {
        n = MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);
        lastBits = MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;
        if (lastBits) {
          *backLen = (n - 1) * 8 + lastBits;
        } else {
          *backLen = n * 8;
        }

        if (n == 0)
          n = 1;
        if (n > 16)
          n = 16;

        for (i = 0; i < n; i++) {
          backData[i] = MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA);
        }
      }
    } else {
      status = MI_ERR;
    }
  }

  return status;
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType) {
  uint8_t status;
  uint16_t backBits;

  MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07); // TxLastBists
  TagType[0] = reqMode;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

  if ((status != MI_OK) || (backBits != 0x10)) {
    status = MI_ERR;
  }
  return status;
}

uint8_t MFRC522_Anticoll(uint8_t *serNum) {
  uint8_t status;
  uint8_t i;
  uint8_t serNumCheck = 0;
  uint16_t unLen;

  MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);

  serNum[0] = PICC_ANTICOLL;
  serNum[1] = 0x20;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

  if (status == MI_OK) {
    for (i = 0; i < 4; i++) {
      serNumCheck ^= serNum[i];
    }
    if (serNumCheck != serNum[4]) {
      status = MI_ERR;
    }
  }
  return status;
}
