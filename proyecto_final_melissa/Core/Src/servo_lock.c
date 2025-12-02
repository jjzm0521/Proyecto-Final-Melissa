#include "servo_lock.h"

// Servo Pulse Widths (in timer ticks, assuming 1MHz clock -> 1us per tick)
// Standard Servo: 0 deg = 1000us, 180 deg = 2000us.
// Or 0 deg = 500us, 180 deg = 2500us.
// Let's assume standard 500-2500 for full range, but safe 1000-2000 for 0-90 might be safer initially.
// Let's use 500us (0.5ms) to 2500us (2.5ms) for 0-180.
#define MIN_PULSE 500
#define MAX_PULSE 2500

void Servo_Init(Servo_t* servo, TIM_HandleTypeDef* htim, uint32_t Channel) {
    servo->htim = htim;
    servo->Channel = Channel;
    HAL_TIM_PWM_Start(servo->htim, servo->Channel);
}

void Servo_SetAngle(Servo_t* servo, uint8_t angle) {
    if (angle > 180) angle = 180;
    
    // Map angle to pulse width
    uint32_t pulse = MIN_PULSE + ((MAX_PULSE - MIN_PULSE) * angle) / 180;
    
    __HAL_TIM_SET_COMPARE(servo->htim, servo->Channel, pulse);
}

void Servo_Open(Servo_t* servo) {
    Servo_SetAngle(servo, SERVO_OPEN_ANGLE);
}

void Servo_Close(Servo_t* servo) {
    Servo_SetAngle(servo, SERVO_CLOSE_ANGLE);
}
