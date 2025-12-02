#ifndef SERVO_LOCK_H
#define SERVO_LOCK_H

#include "stm32f4xx_hal.h"

// Servo Config
#define SERVO_OPEN_ANGLE 90
#define SERVO_CLOSE_ANGLE 0

typedef struct {
    TIM_HandleTypeDef* htim;
    uint32_t Channel;
} Servo_t;

void Servo_Init(Servo_t* servo, TIM_HandleTypeDef* htim, uint32_t Channel);
void Servo_SetAngle(Servo_t* servo, uint8_t angle);
void Servo_Open(Servo_t* servo);
void Servo_Close(Servo_t* servo);

#endif
