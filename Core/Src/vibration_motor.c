#include "vibration_motor.h"

#include "tim.h"

enum { MOTOR_IDLE = 0, MOTOR_START, MOTOR_VIBRATE };

static int8_t motor_L_state = MOTOR_START;
static int8_t motor_R_state = 0;
static int8_t motor_M_state = 0;

static uint32_t lastTime = 0;

void vibration_motor_loop()
{
  if (motor_L_state == MOTOR_START) {
    lastTime = HAL_GetTick();

    htim8.Instance->CCR2 = 254;
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);

    motor_L_state = MOTOR_VIBRATE;
  } else if (motor_L_state == MOTOR_VIBRATE) {
    if (HAL_GetTick() - lastTime > 800) {
      HAL_TIM_PWM_Stop(&htim8, TIM_CHANNEL_2);

      motor_L_state = MOTOR_IDLE;
    }
  }
}