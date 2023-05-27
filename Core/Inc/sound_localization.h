#ifndef __SOUND_LOCALIZATION_H__
#define __SOUND_LOCALIZATION_H__

#include "stm32f4xx_hal.h"

#define ACOUSTIC_SL_M12_DISTANCE 2400
#define ACOUSTIC_SL_M23_DISTANCE 2400
#define ACOUSTIC_SL_M13_DISTANCE 2400

uint32_t SL_Init(void);
void SL_DataInput(int16_t* aPCMBufferIN);
void SL_Process_callback(void);

#endif