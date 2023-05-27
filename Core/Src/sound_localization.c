#include "sound_localization.h"

#include <stdlib.h>

#include "acoustic_sl.h"
#include "main.h"

/*Handler and Config structure for Source Localization*/
static AcousticSL_Handler_t libSoundSourceLoc_Handler_Instance;
static AcousticSL_Config_t libSoundSourceLoc_Config_Instance;

static int32_t estimatedAngle;

uint32_t SL_Init(void)
{
  uint32_t error_value = 0;
  /*Setup Source Localization static parameters*/
  libSoundSourceLoc_Handler_Instance.channel_number = 2;
  libSoundSourceLoc_Handler_Instance.M12_distance = ACOUSTIC_SL_M12_DISTANCE;
  libSoundSourceLoc_Handler_Instance.M34_distance = ACOUSTIC_SL_M12_DISTANCE;  // Unused
  libSoundSourceLoc_Handler_Instance.sampling_frequency = 16000;
  libSoundSourceLoc_Handler_Instance.algorithm = ACOUSTIC_SL_ALGORITHM_BMPH;
  libSoundSourceLoc_Handler_Instance.ptr_M1_channels = 2;
  libSoundSourceLoc_Handler_Instance.ptr_M2_channels = 2;
  libSoundSourceLoc_Handler_Instance.ptr_M3_channels = 2;
  libSoundSourceLoc_Handler_Instance.ptr_M4_channels = 2;
  libSoundSourceLoc_Handler_Instance.samples_to_process = 256;
  AcousticSL_getMemorySize(&libSoundSourceLoc_Handler_Instance);
  libSoundSourceLoc_Handler_Instance.pInternalMemory =
      (uint32_t*)malloc(libSoundSourceLoc_Handler_Instance.internal_memory_size);

  if (libSoundSourceLoc_Handler_Instance.pInternalMemory == NULL) {
    Error_Handler();
  }

  error_value = AcousticSL_Init(&libSoundSourceLoc_Handler_Instance);

  if (error_value != 0) {
    Error_Handler();
  }

  /*Setup Source Localization dynamic parameters*/
  libSoundSourceLoc_Config_Instance.threshold = 5;
  libSoundSourceLoc_Config_Instance.resolution = 4;
  error_value = AcousticSL_setConfig(&libSoundSourceLoc_Handler_Instance, &libSoundSourceLoc_Config_Instance);

  if (error_value != 0) {
    Error_Handler();
  }

  /* SL process task*/
  HAL_NVIC_SetPriority((IRQn_Type)EXTI2_IRQn, 13, 0);
  HAL_NVIC_EnableIRQ((IRQn_Type)EXTI2_IRQn);

  return error_value;
}

void SL_DataInput(int16_t* aPCMBufferIN)
{
  if (AcousticSL_Data_Input((int16_t*)&aPCMBufferIN[0], (int16_t*)&aPCMBufferIN[1], NULL, NULL,
                            &libSoundSourceLoc_Handler_Instance)) {
    HAL_NVIC_SetPendingIRQ(EXTI2_IRQn);
  }
}

void SL_Process_callback(void)
{
  AcousticSL_Process(&estimatedAngle, &libSoundSourceLoc_Handler_Instance);

  // if (estimatedAngle != ACOUSTIC_SL_NO_AUDIO_DETECTED) {
  //   estimatedAngle = (estimatedAngle - 45);
  //   if (estimatedAngle < 0) {
  //     estimatedAngle += 360;
  //   }
  // }
}