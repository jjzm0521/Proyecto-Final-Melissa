#include "state_machine.h"
#include "main.h"
#include "rc522.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Configuration
#define CODE_LENGTH 4
#define AUTO_CLOSE_DELAY 5000 // ms
#define MAX_LEN 16

// State Variables
static SystemState_t currentState = STATE_IDLE;
static char currentCode[CODE_LENGTH + 1];
static uint8_t codeIndex = 0;
static char savedPassword[CODE_LENGTH + 1] = "1234"; // Contraseña por defecto
static uint32_t stateEntryTime = 0;
static uint8_t failedAttempts = 0;
static const uint8_t AUTHORIZED_UID[4] = {0xDE, 0xAD, 0xBE,
                                          0xEF}; // Change as needed

// Hardware Handles
static LiquidCrystal_I2C_t *lcdHandle;
static Keypad_t *keypadHandle;
static Servo_t *servoHandle;
static UART_HandleTypeDef *uartHandle;
static volatile char uartBuffer = 0; // Buffer for ISR

// Helper Functions
static void TransitionTo(SystemState_t newState);
static void ClearInput(void);
static void SM_Print(const char *str);
static void SM_Clear(void);
static void SM_SetCursor(uint8_t col, uint8_t row);
static void SM_ProcessUART(char key);

void SM_Init(LiquidCrystal_I2C_t *lcd, Keypad_t *keypad, Servo_t *servo,
             UART_HandleTypeDef *huart) {
  lcdHandle = lcd;
  keypadHandle = keypad;
  servoHandle = servo;
  uartHandle = huart;

  SM_Clear();
  SM_Print("Smart Lock Listo");
  SM_SetCursor(0, 1);
  SM_Print("Teclas:0-9 A-D");
  HAL_Delay(1000);

  if (uartHandle != NULL) {
    char *menu = "\r\n--- Menu Smart Lock ---\r\n"
                 "Teclas: 0-9, A-D\r\n"
                 "Cmds: 'U'Abrir, 'L'Cerrar\r\n"
                 "-----------------------\r\n";
    HAL_UART_Transmit(uartHandle, (uint8_t *)menu, strlen(menu), 1000);
  }

  TransitionTo(STATE_IDLE);
}

void SM_Run(void) {
  // 1. Check Keypad
  char key = Keypad_GetKey(keypadHandle);
  if (key) {
    SM_HandleKey(key);
  }

  // 1.5 Check UART Buffer (Process out of ISR)
  if (uartBuffer != 0) {
    char k = uartBuffer;
    uartBuffer = 0;
    SM_ProcessUART(k);
  }

  // 2. Check Card (only in IDLE)
  if (currentState == STATE_IDLE) {
    if (SM_CheckCard()) {
      TransitionTo(STATE_ACCESS_GRANTED);
    }
  }

  // 3. Timeouts / Auto-actions
  uint32_t elapsed = HAL_GetTick() - stateEntryTime;

  switch (currentState) {
  case STATE_ACCESS_GRANTED:
    if (elapsed > AUTO_CLOSE_DELAY) {
      TransitionTo(STATE_IDLE);
    }
    break;

  case STATE_ACCESS_DENIED:
    if (elapsed > 2000) {
      TransitionTo(STATE_IDLE);
    }
    break;

  case STATE_INPUT_CODE:
    if (elapsed > 10000) { // 10s timeout for input
      TransitionTo(STATE_IDLE);
    }
    break;

  case STATE_BLOCKED:
    if (elapsed > 30000) { // 30s block
      failedAttempts = 0;
      TransitionTo(STATE_IDLE);
    }
    break;

  default:
    break;
  }
}

void SM_HandleKey(char key) {
  if (currentState == STATE_BLOCKED)
    return; // Ignore keys when blocked

  switch (currentState) {
  case STATE_IDLE:
    if (key == 'A') {
      TransitionTo(STATE_CHANGE_PWD_AUTH);
    } else {
      TransitionTo(STATE_INPUT_CODE);
      currentCode[codeIndex++] = key;
      SM_Print("*");
    }
    break;

  case STATE_INPUT_CODE:
    if (codeIndex < CODE_LENGTH) {
      currentCode[codeIndex++] = key;
      SM_Print("*");
    }

    if (codeIndex >= CODE_LENGTH) {
      currentCode[CODE_LENGTH] = '\0';
      TransitionTo(STATE_CHECK_CODE);
    }
    break;

  case STATE_CHANGE_PWD_AUTH:
    if (codeIndex < CODE_LENGTH) {
      currentCode[codeIndex++] = key;
      SM_Print("*");
    }
    if (codeIndex >= CODE_LENGTH) {
      currentCode[CODE_LENGTH] = '\0';
      if (strcmp(currentCode, savedPassword) == 0) {
        TransitionTo(STATE_CHANGE_PWD_NEW);
      } else {
        TransitionTo(STATE_ACCESS_DENIED);
      }
    }
    break;

  case STATE_CHANGE_PWD_NEW:
    if (codeIndex < CODE_LENGTH) {
      currentCode[codeIndex++] = key;
      SM_Print("*");
    }
    if (codeIndex >= CODE_LENGTH) {
      currentCode[CODE_LENGTH] = '\0';
      strcpy(savedPassword, currentCode);
      TransitionTo(STATE_CHANGE_PWD_CONFIRM);
    }
    break;

  default:
    break;
  }
}

static void TransitionTo(SystemState_t newState) {
  currentState = newState;
  stateEntryTime = HAL_GetTick();

  switch (newState) {
  case STATE_IDLE:
    Servo_Close(servoHandle);
    SM_Clear();
    SM_SetCursor(0, 0);
    SM_Print("CERRADO");
    ClearInput();
    break;

  case STATE_INPUT_CODE:
    SM_Clear();
    SM_Print("Ingrese Codigo: ");
    SM_SetCursor(0, 1);
    // If we came from IDLE with a key press, that key is already handled in
    // HandleKey but we need to reprint it if we cleared screen. Actually,
    // HandleKey calls TransitionTo FIRST, then prints. So if we transition
    // here, the screen is clear. The key that triggered IDLE->INPUT will be
    // printed by HandleKey *after* this returns.
    break;

  case STATE_CHECK_CODE:
    if (strcmp(currentCode, savedPassword) == 0) {
      failedAttempts = 0;
      TransitionTo(STATE_ACCESS_GRANTED);
    } else {
      failedAttempts++;
      if (failedAttempts >= 3) {
        TransitionTo(STATE_BLOCKED);
      } else {
        TransitionTo(STATE_ACCESS_DENIED);
      }
    }
    break;

  case STATE_ACCESS_GRANTED:
    Servo_Open(servoHandle);
    SM_Clear();
    SM_Print("ABIERTO");
    break;

  case STATE_ACCESS_DENIED:
    SM_Clear();
    SM_Print("Acceso Denegado");
    SM_SetCursor(0, 1);
    char buf[16];
    sprintf(buf, "Intentos: %d/3", failedAttempts);
    SM_Print(buf);
    break;

  case STATE_BLOCKED:
    SM_Clear();
    SM_Print("SISTEMA BLOQ.");
    SM_SetCursor(0, 1);
    SM_Print("Espere 30s...");
    break;

  case STATE_CHANGE_PWD_AUTH:
    ClearInput();
    SM_Clear();
    SM_Print("Antigua Clave: ");
    SM_SetCursor(0, 1);
    break;

  case STATE_CHANGE_PWD_NEW:
    ClearInput();
    SM_Clear();
    SM_Print("Nueva Clave: ");
    SM_SetCursor(0, 1);
    break;

  case STATE_CHANGE_PWD_CONFIRM:
    SM_Clear();
    SM_Print("Cambiada!");
    HAL_Delay(1000); // Blocking delay for simplicity
    TransitionTo(STATE_IDLE);
    break;
  }
}

static void ClearInput(void) {
  codeIndex = 0;
  memset(currentCode, 0, sizeof(currentCode));
}

bool SM_CheckCard(void) {
  uint8_t status;
  uint8_t str[MAX_LEN]; // Ensure MAX_LEN is defined or use 16

  // 1. Request Card
  status = MFRC522_Request(PICC_REQIDL, str);
  if (status == MI_OK) {
    // 2. Anti-collision (Get UID)
    status = MFRC522_Anticoll(str);
    if (status == MI_OK) {
      // Card Detected!
      // str[0]..str[3] is the UID.

      // Debug Print UID
      char uidStr[32];
      sprintf(uidStr, "UID: %02X%02X%02X%02X", str[0], str[1], str[2], str[3]);
      SM_Print(uidStr);
      HAL_Delay(1000);

      // Verify UID
      bool authorized = true;
      for (int i = 0; i < 4; i++) {
        if (str[i] != AUTHORIZED_UID[i]) {
          authorized = false;
          break;
        }
      }

      if (authorized) {
        return true;
      } else {
        SM_Clear();
        SM_Print("No Autorizado");
        HAL_Delay(2000);
        TransitionTo(STATE_IDLE); // Return to idle to clear screen
        return false;
      }
    }
  }
  return false;
}

static void SM_Print(const char *str) {
  LiquidCrystal_I2C_print(lcdHandle, (char *)str);
  if (uartHandle != NULL) {
    HAL_UART_Transmit(uartHandle, (uint8_t *)str, strlen(str), 1000);
  }
}

static void SM_Clear(void) {
  LiquidCrystal_I2C_clear(lcdHandle);
  if (uartHandle != NULL) {
    // ANSI Clear Screen REMOVED for scrolling log
    // Just print a separator or newline
    const char *sep = "\r\n----------------\r\n";
    HAL_UART_Transmit(uartHandle, (uint8_t *)sep, strlen(sep), 1000);
  }
}

static void SM_SetCursor(uint8_t col, uint8_t row) {
  LiquidCrystal_I2C_setCursor(lcdHandle, col, row);
  if (uartHandle != NULL) {
    // ANSI Set Cursor REMOVED for scrolling log
    // If moving to 2nd line (row > 0), just print newline
    if (row > 0) {
      const char *nl = "\r\n";
      HAL_UART_Transmit(uartHandle, (uint8_t *)nl, strlen(nl), 1000);
    }
  }
}

void SM_HandleUART(char key) {
  uartBuffer = key; // Just buffer it, don't process (LCD/I2C) in ISR!
}

static void SM_ProcessUART(char key) {
  if (key == 'U') {
    TransitionTo(STATE_ACCESS_GRANTED);
  } else if (key == 'L') {
  } else {
    SM_HandleKey(key);
  }
}
