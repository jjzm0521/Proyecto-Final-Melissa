/*
 * rc522.h
 * Adaptado de MFRC522.h provisto
 */

#ifndef RC522_H_
#define RC522_H_

#include "main.h" // Importante para tener acceso a HAL y definiciones de pines
#include <stdint.h>

// --- Definiciones de Hardware ---
// Asegúrate de que SPI1 esté definido en main.c
extern SPI_HandleTypeDef hspi1;

// --- Comandos MFRC522 ---
#define PCD_IDLE 0x00
#define PCD_AUTHENT 0x0E
#define PCD_RECEIVE 0x08
#define PCD_TRANSMIT 0x04
#define PCD_TRANSCEIVE 0x0C
#define PCD_RESETPHASE 0x0F
#define PCD_CALCCRC 0x03

// --- Comandos MIFARE ---
#define PICC_REQIDL 0x26
#define PICC_REQALL 0x52
#define PICC_ANTICOLL 0x93
#define PICC_SElECTTAG 0x93
#define PICC_HALT 0x50

// --- Códigos de Retorno ---
#define MI_OK 0
#define MI_NOTAGERR 1
#define MI_ERR 2

// --- Registros MFRC522 (Selección de los más usados) ---
// Page 0: Command and Status
#define MFRC522_REG_RESERVED00 0x00
#define MFRC522_REG_COMMAND 0x01
#define MFRC522_REG_COMM_IE_N 0x02
#define MFRC522_REG_DIV1_EN 0x03
#define MFRC522_REG_COMM_IRQ 0x04
#define MFRC522_REG_DIV_IRQ 0x05
#define MFRC522_REG_ERROR 0x06
#define MFRC522_REG_STATUS1 0x07
#define MFRC522_REG_STATUS2 0x08
#define MFRC522_REG_FIFO_DATA 0x09
#define MFRC522_REG_FIFO_LEVEL 0x0A
#define MFRC522_REG_WATER_LEVEL 0x0B
#define MFRC522_REG_CONTROL 0x0C
#define MFRC522_REG_BIT_FRAMING 0x0D
#define MFRC522_REG_COLL 0x0E
#define MFRC522_REG_RESERVED01 0x0F

// Page 1: Command
#define MFRC522_REG_MODE 0x11
#define MFRC522_REG_TX_MODE 0x12
#define MFRC522_REG_RX_MODE 0x13
#define MFRC522_REG_TX_CONTROL 0x14
#define MFRC522_REG_TX_AUTO 0x15
#define MFRC522_REG_TX_SELL 0x16
#define MFRC522_REG_RX_SELL 0x17
#define MFRC522_REG_RX_THRESHOLD 0x18
#define MFRC522_REG_DEMOD 0x19
#define MFRC522_REG_MIFARE 0x1C
#define MFRC522_REG_SERIAL_SPEED 0x1F

// Page 2: CFG
#define MFRC522_REG_CRC_RESULT_M 0x21
#define MFRC522_REG_CRC_RESULT_L 0x22
#define MFRC522_REG_MOD_WIDTH 0x24
#define MFRC522_REG_RF_CFG 0x26
#define MFRC522_REG_GS_N 0x27
#define MFRC522_REG_CWGS_P 0x28
#define MFRC522_REG_MODGS_P 0x29
#define MFRC522_REG_T_MODE 0x2A
#define MFRC522_REG_T_PRESCALER 0x2B
#define MFRC522_REG_T_RELOAD_H 0x2C
#define MFRC522_REG_T_RELOAD_L 0x2D
#define MFRC522_REG_T_COUNTER_VALUE_H 0x2E
#define MFRC522_REG_T_COUNTER_VALUE_L 0x2F

// Page 3: TestRegister
#define MFRC522_REG_VERSION 0x37

// --- Prototipos de Funciones ---
void MFRC522_Init(void);
void MFRC522_Reset(void);
void MFRC522_AntennaOn(void);
void MFRC522_AntennaOff(void);
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType);
uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen,
                       uint8_t *backData, uint16_t *backLen);
uint8_t MFRC522_Anticoll(uint8_t *serNum);
void MFRC522_WriteRegister(uint8_t addr, uint8_t val);
uint8_t MFRC522_ReadRegister(uint8_t addr);

#endif /* RC522_H_ */
