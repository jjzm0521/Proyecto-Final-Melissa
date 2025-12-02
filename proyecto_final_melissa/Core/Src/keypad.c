#include "keypad.h"

// Keypad Layout
const char KEYMAP[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void Keypad_Init(Keypad_t* keypad, TIM_HandleTypeDef* htim) {
    keypad->Timer = htim;
    keypad->currentColumn = 0;
    keypad->newKeyAvailable = false;
    keypad->lastKey = 0;
    keypad->lastDebounceTime = 0;

    // Initialize Columns: All HIGH (Inactive) initially
    for (int i = 0; i < KEYPAD_COLS; i++) {
        HAL_GPIO_WritePin(keypad->ColPorts[i], keypad->ColPins[i], GPIO_PIN_SET);
    }
    
    // Start with Column 0 Active (LOW)
    HAL_GPIO_WritePin(keypad->ColPorts[0], keypad->ColPins[0], GPIO_PIN_RESET);

    // Start Timer for cycling columns
    HAL_TIM_Base_Start_IT(keypad->Timer);
}

char Keypad_GetKey(Keypad_t* keypad) {
    if (keypad->newKeyAvailable) {
        keypad->newKeyAvailable = false;
        return keypad->lastKey;
    }
    return 0;
}

// Called by Timer ISR (e.g., every 20ms)
void Keypad_TimerTick(Keypad_t* keypad) {
    // 1. Deactivate current column (Set HIGH)
    HAL_GPIO_WritePin(keypad->ColPorts[keypad->currentColumn], keypad->ColPins[keypad->currentColumn], GPIO_PIN_SET);

    // 2. Move to next column
    keypad->currentColumn++;
    if (keypad->currentColumn >= KEYPAD_COLS) {
        keypad->currentColumn = 0;
    }

    // 3. Activate new column (Set LOW)
    HAL_GPIO_WritePin(keypad->ColPorts[keypad->currentColumn], keypad->ColPins[keypad->currentColumn], GPIO_PIN_RESET);
}

// Called by EXTI ISR
void Keypad_HandleInterrupt(Keypad_t* keypad, uint16_t GPIO_Pin) {
    // Simple Debounce: Ignore interrupts if too close to the last one
    uint32_t now = HAL_GetTick();
    if (now - keypad->lastDebounceTime < 200) {
        return;
    }

    // Identify Row
    int row = -1;
    for (int i = 0; i < KEYPAD_ROWS; i++) {
        if (GPIO_Pin == keypad->RowPins[i]) {
            row = i;
            break;
        }
    }

    if (row != -1) {
        // We know the Row (from Pin) and the Column (from currentColumn)
        // Note: Since we cycle columns, if we are here, it means the Row is LOW
        // AND the current active Column is LOW. So this is a valid intersection.
        
        // Double check: Is the Row actually LOW? (Falling edge trigger implies it went low)
        if (HAL_GPIO_ReadPin(keypad->RowPorts[row], keypad->RowPins[row]) == GPIO_PIN_RESET) {
            keypad->lastKey = KEYMAP[row][keypad->currentColumn];
            keypad->newKeyAvailable = true;
            keypad->lastDebounceTime = now;
        }
    }
}
