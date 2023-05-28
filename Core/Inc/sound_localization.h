#ifndef __SOUND_LOCALIZATION_H__
#define __SOUND_LOCALIZATION_H__

#include "stm32f4xx_hal.h"

#define ACOUSTIC_SL_M12_DISTANCE 1600
#define ACOUSTIC_SL_M23_DISTANCE 1600
#define ACOUSTIC_SL_M13_DISTANCE 1430

uint32_t SL_Init(void);
void SL_DataInput(int16_t* aPCMBufferIN);
void SL_Process_callback(void);
float SL_XCORR_GetAngle(int16_t* M1_data, int16_t* M2_data, size_t dataSize);

#endif