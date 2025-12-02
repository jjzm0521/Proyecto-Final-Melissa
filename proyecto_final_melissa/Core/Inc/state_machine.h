#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include "LiquidCrystal_I2C.h"
#include "keypad.h"
#include "servo_lock.h"

typedef enum {
    STATE_IDLE,
    STATE_INPUT_CODE,
    STATE_CHECK_CODE,
    STATE_ACCESS_GRANTED,
    STATE_ACCESS_DENIED,
    STATE_CHANGE_PWD_AUTH,
    STATE_CHANGE_PWD_NEW,
    STATE_CHANGE_PWD_CONFIRM
} SystemState_t;

void SM_Init(LiquidCrystal_I2C_t* lcd, Keypad_t* keypad, Servo_t* servo);
void SM_Run(void);
void SM_HandleKey(char key);
bool SM_CheckCard(void); // Placeholder for Card/RFID

#endif
