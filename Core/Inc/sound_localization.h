#ifndef __SOUND_LOCALIZATION_H__
#define __SOUND_LOCALIZATION_H__

#include "stm32f4xx_hal.h"

#define ACOUSTIC_SL_M12_DISTANCE 0.15836f
#define ACOUSTIC_SL_M23_DISTANCE 0.15836f
#define ACOUSTIC_SL_M31_DISTANCE 0.14304f

float SL_XCORR_GetAngle(int16_t* M1_data, int16_t* M2_data, int16_t* M3_data, size_t dataSize);

int SL_get_direction();

#endif