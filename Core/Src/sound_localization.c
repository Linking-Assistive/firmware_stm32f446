#include "sound_localization.h"

#include <arm_math.h>
#include <stdlib.h>

#include "acoustic_sl.h"
#include "audio_sample.h"
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

float32_t SL_XCORR_GetAngle(int16_t* M1_data, int16_t* M2_data, size_t dataSize)
{
  int32_t tau, k;
  volatile int32_t delta_t13 = 0;

  int64_t correlation = 0;
  int64_t MAX13 = 0;

  float32_t angle = 0.0;

  float32_t distance = ACOUSTIC_SL_M12_DISTANCE / 10000.0f;
  int32_t M12_TAUD = (int32_t)(distance * (float32_t)AUDIO_IN_SAMPLING_FREQUENCY / (float32_t)343.1f);

  volatile int64_t history[15];

  // MAX13 = 0x80000000U;  //-inf
  for (tau = -M12_TAUD; tau <= M12_TAUD; tau++) {
    correlation = 0;
    for (k = M12_TAUD; k < dataSize - M12_TAUD; k++) {
      correlation += (int32_t)M2_data[k] * (int32_t)M1_data[k + tau];
    }
    history[tau + M12_TAUD] = correlation;
    if (correlation > MAX13) {
      MAX13 = correlation;
      delta_t13 = tau;
    }
  }

  // if (SLocInternal->Mic_Number == 4U) {
  //   MAX24 = 0x80000000U;  //-inf
  //   for (tau = -SLocInternal->M34_TAUD; tau <= SLocInternal->M34_TAUD; tau++) {
  //     correlation = 0;
  //     for (k = SLocInternal->M34_TAUD; k < ((int32_t)SLocInternal->Sample_Number_To_Process -
  //     SLocInternal->M34_TAUD);
  //          k++) {
  //       correlation =
  //           correlation + (((uint32_t)((int16_t*)(SLocInternal->M4_Data))[((SLocInternal->Buffer_State - 1U) *
  //                                                                          SLocInternal->Sample_Number_To_Process) +
  //                                                                         (uint32_t)k] *
  //                           (uint32_t)((int16_t*)(SLocInternal->M3_Data))[((SLocInternal->Buffer_State - 1U) *
  //                                                                          SLocInternal->Sample_Number_To_Process) +
  //                                                                         (uint32_t)k + (uint32_t)tau]) /
  //                          256U);
  //     }
  //     if (correlation > MAX24) {
  //       MAX24 = correlation;
  //       delta_t24 = (float32_t)tau;
  //     }
  //   }
  // }

  angle = (float32_t)(delta_t13 + M12_TAUD) * 90.0f / (float32_t)M12_TAUD;

  // if (SLocInternal->Mic_Number == 2U) {
  //   test = (delta_t13 + (float32_t)SLocInternal->M12_TAUD) * (180.0f / ((float32_t)SLocInternal->M12_TAUD * 2.0f));
  // } else if (SLocInternal->Mic_Number == 4U) {
  //   test = (float32_t)atan2(delta_t13, delta_t24);
  //   test = test * (360.0f / (3.14f * 2.0f));
  //   if (test < 0.0f) {
  //     test += 360.0f; /* in [0 360] -> TODO: Optimize the whole angle computation from the maximum phase index*/
  //   }
  // } else {
  //   /* no other use cases are handled */
  // }

  return angle;
}
