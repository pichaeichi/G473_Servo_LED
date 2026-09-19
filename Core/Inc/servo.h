#ifndef SERVO_H
#define SERVO_H

#include "main.h"
#include <stdint.h>

/* 本工程将 TIM1 配成 1 MHz 计数，因此比较值的数字等于脉宽微秒数。 */
#define SERVO_MIN_PULSE_US  1000U
#define SERVO_MAX_PULSE_US  2000U
#define SERVO_CENTER_ANGLE    90

HAL_StatusTypeDef Servo_Init(TIM_HandleTypeDef *timer, uint32_t channel);
void Servo_SetAngle(int16_t angle_deg);
void Servo_SetOffset(int16_t offset_deg);
void Servo_Center(void);

#endif
