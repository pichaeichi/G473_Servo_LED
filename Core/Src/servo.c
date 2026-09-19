#include "servo.h"

static TIM_HandleTypeDef *s_servo_timer = NULL;
static uint32_t s_servo_channel = TIM_CHANNEL_1;

static int16_t Servo_ClampAngle(int16_t angle_deg)
{
    if (angle_deg < 0)
    {
        return 0;
    }
    if (angle_deg > 180)
    {
        return 180;
    }
    return angle_deg;
}

HAL_StatusTypeDef Servo_Init(TIM_HandleTypeDef *timer, uint32_t channel)
{
    s_servo_timer = timer;
    s_servo_channel = channel;

    __HAL_TIM_SET_COMPARE(s_servo_timer, s_servo_channel, 1500U);
    return HAL_TIM_PWM_Start(s_servo_timer, s_servo_channel);
}

void Servo_SetAngle(int16_t angle_deg)
{
    uint32_t pulse_us;

    if (s_servo_timer == NULL)
    {
        return;
    }

    angle_deg = Servo_ClampAngle(angle_deg);
    pulse_us = SERVO_MIN_PULSE_US
             + ((uint32_t)angle_deg * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) / 180U;

    __HAL_TIM_SET_COMPARE(s_servo_timer, s_servo_channel, pulse_us);
}

void Servo_SetOffset(int16_t offset_deg)
{
    Servo_SetAngle((int16_t)(SERVO_CENTER_ANGLE + offset_deg));
}

void Servo_Center(void)
{
    Servo_SetAngle(SERVO_CENTER_ANGLE);
}
