#include "state_machine.h"
#include <string.h>
#include <stdio.h>

// Configuration
#define CODE_LENGTH 4
#define AUTO_CLOSE_DELAY 5000 // ms

// State Variables
static SystemState_t currentState = STATE_IDLE;
static char currentCode[CODE_LENGTH + 1];
static uint8_t codeIndex = 0;
static char savedPassword[CODE_LENGTH + 1] = "1234"; // Default Password
static uint32_t stateEntryTime = 0;

// Hardware Handles
static LiquidCrystal_I2C_t* lcdHandle;
static Keypad_t* keypadHandle;
static Servo_t* servoHandle;

// Helper Functions
static void TransitionTo(SystemState_t newState);
static void ClearInput(void);

void SM_Init(LiquidCrystal_I2C_t* lcd, Keypad_t* keypad, Servo_t* servo) {
    lcdHandle = lcd;
    keypadHandle = keypad;
    servoHandle = servo;
    
    LiquidCrystal_I2C_clear(lcdHandle);
    TransitionTo(STATE_IDLE);
}

void SM_Run(void) {
    // 1. Check Keypad
    char key = Keypad_GetKey(keypadHandle);
    if (key) {
        SM_HandleKey(key);
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
            
        default:
            break;
    }
}

void SM_HandleKey(char key) {
    switch (currentState) {
        case STATE_IDLE:
            if (key == 'A') {
                TransitionTo(STATE_CHANGE_PWD_AUTH);
            } else {
                TransitionTo(STATE_INPUT_CODE);
                currentCode[codeIndex++] = key;
                LiquidCrystal_I2C_print(lcdHandle, "*");
            }
            break;
            
        case STATE_INPUT_CODE:
            if (codeIndex < CODE_LENGTH) {
                currentCode[codeIndex++] = key;
                LiquidCrystal_I2C_print(lcdHandle, "*");
            }
            
            if (codeIndex >= CODE_LENGTH) {
                currentCode[CODE_LENGTH] = '\0';
                TransitionTo(STATE_CHECK_CODE);
            }
            break;
            
        case STATE_CHANGE_PWD_AUTH:
            if (codeIndex < CODE_LENGTH) {
                currentCode[codeIndex++] = key;
                LiquidCrystal_I2C_print(lcdHandle, "*");
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
                LiquidCrystal_I2C_print(lcdHandle, "*");
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
            LiquidCrystal_I2C_clear(lcdHandle);
            LiquidCrystal_I2C_setCursor(lcdHandle, 0, 0);
            LiquidCrystal_I2C_print(lcdHandle, "LOCKED");
            ClearInput();
            break;
            
        case STATE_INPUT_CODE:
            LiquidCrystal_I2C_clear(lcdHandle);
            LiquidCrystal_I2C_print(lcdHandle, "Enter Code:");
            LiquidCrystal_I2C_setCursor(lcdHandle, 0, 1);
            // If we came from IDLE with a key press, that key is already handled in HandleKey
            // but we need to reprint it if we cleared screen.
            // Actually, HandleKey calls TransitionTo FIRST, then prints.
            // So if we transition here, the screen is clear.
            // The key that triggered IDLE->INPUT will be printed by HandleKey *after* this returns.
            break;
            
        case STATE_CHECK_CODE:
            if (strcmp(currentCode, savedPassword) == 0) {
                TransitionTo(STATE_ACCESS_GRANTED);
            } else {
                TransitionTo(STATE_ACCESS_DENIED);
            }
            break;
            
        case STATE_ACCESS_GRANTED:
            Servo_Open(servoHandle);
            LiquidCrystal_I2C_clear(lcdHandle);
            LiquidCrystal_I2C_print(lcdHandle, "OPEN");
            break;
            
        case STATE_ACCESS_DENIED:
            LiquidCrystal_I2C_clear(lcdHandle);
            LiquidCrystal_I2C_print(lcdHandle, "ERROR");
            break;
            
        case STATE_CHANGE_PWD_AUTH:
            ClearInput();
            LiquidCrystal_I2C_clear(lcdHandle);
            LiquidCrystal_I2C_print(lcdHandle, "Old Pass:");
            LiquidCrystal_I2C_setCursor(lcdHandle, 0, 1);
            break;
            
        case STATE_CHANGE_PWD_NEW:
            ClearInput();
            LiquidCrystal_I2C_clear(lcdHandle);
            LiquidCrystal_I2C_print(lcdHandle, "New Pass:");
            LiquidCrystal_I2C_setCursor(lcdHandle, 0, 1);
            break;
            
        case STATE_CHANGE_PWD_CONFIRM:
            LiquidCrystal_I2C_clear(lcdHandle);
            LiquidCrystal_I2C_print(lcdHandle, "Changed!");
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
    // TODO: Implement Card/RFID check
    return false;
}
