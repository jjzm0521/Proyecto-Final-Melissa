#ifndef KEYPAD_H
#define KEYPAD_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// Define Keypad Dimensions
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

// Keypad Structure
typedef struct {
    GPIO_TypeDef* RowPorts[KEYPAD_ROWS];
    uint16_t RowPins[KEYPAD_ROWS];
    GPIO_TypeDef* ColPorts[KEYPAD_COLS];
    uint16_t ColPins[KEYPAD_COLS];
    TIM_HandleTypeDef* Timer; // Timer for scanning
    
    // Internal State
    uint8_t currentColumn;
    volatile char lastKey;
    volatile bool newKeyAvailable;
    uint32_t lastDebounceTime;
} Keypad_t;

// Function Prototypes
void Keypad_Init(Keypad_t* keypad, TIM_HandleTypeDef* htim);
char Keypad_GetKey(Keypad_t* keypad);
void Keypad_HandleInterrupt(Keypad_t* keypad, uint16_t GPIO_Pin); // Call in HAL_GPIO_EXTI_Callback
void Keypad_TimerTick(Keypad_t* keypad); // Call in HAL_TIM_PeriodElapsedCallback

#endif
