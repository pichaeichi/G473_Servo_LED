#include "app.h"
#include "button.h"
#include "servo.h"
#include "tim.h"

#define LED_TIMER_COUNTS_PER_MS  10U
#define FREQUENCY_OPTION_COUNT    4U
#define AMPLITUDE_OPTION_COUNT    3U

/* 周期依次为 0.5 Hz、1 Hz、2 Hz、4 Hz；初始索引 1，即 1 Hz。 */
static const uint16_t s_blink_period_ms[FREQUENCY_OPTION_COUNT] = {2000U, 1000U, 500U, 250U};
static const uint8_t s_frequency_x10[FREQUENCY_OPTION_COUNT] = {5U, 10U, 20U, 40U};
static const uint8_t s_amplitude_deg[AMPLITUDE_OPTION_COUNT] = {20U, 40U, 60U};

static Button s_key;
static uint8_t s_frequency_index = 1U;
static uint8_t s_amplitude_index = 0U;
static uint8_t s_servo_positive = 0U;

volatile AppStatus g_app_status =
{
    .enabled = 1U,
    .led_on = 0U,
    .frequency_x10 = 10U,
    .amplitude_deg = 20U,
    .servo_angle_deg = SERVO_CENTER_ANGLE
};

static void App_SetLed(uint8_t on)
{
    g_app_status.led_on = (on != 0U) ? 1U : 0U;
    HAL_GPIO_WritePin(LED_RUN_GPIO_Port,
                      LED_RUN_Pin,
                      (g_app_status.led_on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void App_ConfigureBlinkTimer(void)
{
    const uint32_t half_period_ms = s_blink_period_ms[s_frequency_index] / 2U;
    const uint32_t auto_reload = half_period_ms * LED_TIMER_COUNTS_PER_MS - 1U;

    (void)HAL_TIM_Base_Stop_IT(&htim7);
    __HAL_TIM_SET_AUTORELOAD(&htim7, auto_reload);
    __HAL_TIM_SET_COUNTER(&htim7, 0U);
    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);

    if (g_app_status.enabled != 0U)
    {
        if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

static void App_StopMotion(void)
{
    (void)HAL_TIM_Base_Stop_IT(&htim7);
    App_SetLed(0U);
    Servo_Center();
    g_app_status.servo_angle_deg = SERVO_CENTER_ANGLE;
    s_servo_positive = 0U;
}

static void App_StartMotion(void)
{
    App_SetLed(0U);
    Servo_Center();
    g_app_status.servo_angle_deg = SERVO_CENTER_ANGLE;
    s_servo_positive = 0U;
    App_ConfigureBlinkTimer();
}

static void App_HandleButtonEvents(uint8_t events)
{
    if ((events & BUTTON_EVENT_LONG) != 0U)
    {
        g_app_status.enabled ^= 1U;
        if (g_app_status.enabled != 0U)
        {
            App_StartMotion();
        }
        else
        {
            App_StopMotion();
        }
    }

    if ((events & BUTTON_EVENT_DOUBLE) != 0U)
    {
        s_amplitude_index = (uint8_t)((s_amplitude_index + 1U) % AMPLITUDE_OPTION_COUNT);
        g_app_status.amplitude_deg = s_amplitude_deg[s_amplitude_index];
    }

    if ((events & BUTTON_EVENT_SHORT) != 0U)
    {
        s_frequency_index = (uint8_t)((s_frequency_index + 1U) % FREQUENCY_OPTION_COUNT);
        g_app_status.frequency_x10 = s_frequency_x10[s_frequency_index];
        App_ConfigureBlinkTimer();
    }
}

void App_Init(void)
{
    App_SetLed(0U);
    Button_Init(&s_key, KEY_GPIO_Port, KEY_Pin, GPIO_PIN_RESET);

    if (Servo_Init(&htim1, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    Servo_Center();

    if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
    {
        Error_Handler();
    }

    App_ConfigureBlinkTimer();
}

void App_Process(void)
{
    const uint8_t events = Button_GetEvents(&s_key);

    if (events != BUTTON_EVENT_NONE)
    {
        App_HandleButtonEvents(events);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        Button_Update1ms(&s_key);
    }
    else if ((htim->Instance == TIM7) && (g_app_status.enabled != 0U))
    {
        int16_t angle;

        App_SetLed((uint8_t)(g_app_status.led_on == 0U));
        s_servo_positive ^= 1U;

        angle = (int16_t)(SERVO_CENTER_ANGLE
                         + ((s_servo_positive != 0U)
                            ? (int16_t)g_app_status.amplitude_deg
                            : -(int16_t)g_app_status.amplitude_deg));

        Servo_SetAngle(angle);
        g_app_status.servo_angle_deg = angle;
    }
}
