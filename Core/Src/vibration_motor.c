#include "vibration_motor.h"

#include "tim.h"
//
#include "SEGGER_SYSVIEW.h"

enum { MOTOR_IDLE = 0, MOTOR_START, MOTOR_VIBRATE };

typedef struct {
  uint8_t motor_id;
  uint8_t state;
  uint32_t strength;
  TIM_HandleTypeDef* htim;
  uint32_t channel;
  volatile uint32_t* CCR; /*!< TIM capture/compare register address */

  uint32_t vibrateDuration;
  uint32_t lastTime;
} motor_t;

static motor_t motors[MOTOR_NUM] = {
    {MOTOR_L, MOTOR_IDLE, 100, &htim8, TIM_CHANNEL_2, &(TIM8->CCR2), 600, 0},
    {MOTOR_M, MOTOR_IDLE, 100, &htim12, TIM_CHANNEL_1, &(TIM12->CCR1), 600, 0},
    {MOTOR_R, MOTOR_IDLE, 100, &htim3, TIM_CHANNEL_1, &(TIM3->CCR1), 600, 0},
};

static void motor_state_transition(motor_t* pMotor)
{
  switch (pMotor->state) {
    case MOTOR_IDLE:
      break;
    case MOTOR_START:
      SEGGER_SYSVIEW_MarkStart(1);
      pMotor->lastTime = HAL_GetTick();

      *pMotor->CCR = pMotor->strength;
      HAL_TIM_PWM_Start(pMotor->htim, pMotor->channel);

      pMotor->state = MOTOR_VIBRATE;
      break;
    case MOTOR_VIBRATE:
      if (HAL_GetTick() - pMotor->lastTime > pMotor->vibrateDuration) {
        SEGGER_SYSVIEW_MarkStop(1);
        HAL_TIM_PWM_Stop(pMotor->htim, pMotor->channel);

        pMotor->state = MOTOR_IDLE;
      }
      break;
    default:
      break;
  }
}

void vibration_motor_loop()
{
  for (int i = 0; i < MOTOR_NUM; i++) {
    motor_state_transition(&motors[i]);
  }
}

#define STRENGTH_LOW         0.0f
#define STRENGTH_HIGH        100.0f
#define STRENGTH_MAP_TO_LOW  100.0f
#define STRENGTH_MAP_TO_HIGH 256.0f

void vibration_motor_start(uint8_t motor_id, uint8_t strength)
{
  if (motor_id > MOTOR_NUM - 1) {
    return;
  }

  if (strength < STRENGTH_LOW) {
    strength = STRENGTH_LOW;
  } else if (strength > STRENGTH_HIGH) {
    strength = STRENGTH_HIGH;
  }

  motors[motor_id].strength =
      ((float)strength - STRENGTH_LOW) / (STRENGTH_HIGH - STRENGTH_LOW) * (STRENGTH_MAP_TO_HIGH - STRENGTH_MAP_TO_LOW) +
      STRENGTH_MAP_TO_LOW;

  motors[motor_id].state = MOTOR_START;
}