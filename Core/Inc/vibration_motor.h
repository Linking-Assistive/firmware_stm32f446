#ifndef __VIBRATION_MOTOR_H
#define __VIBRATION_MOTOR_H

#include "stm32f4xx_hal.h"

enum { MOTOR_L = 0, MOTOR_M, MOTOR_R, MOTOR_NUM };

void vibration_motor_loop(void);
void vibration_motor_start(uint8_t motor_id, uint8_t strength);

#endif