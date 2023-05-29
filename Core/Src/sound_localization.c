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
  libSoundSourceLoc_Handler_Instance.M12_distance = 16000;
  libSoundSourceLoc_Handler_Instance.M34_distance = 16000;  // Unused
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

static float smallest_angular_distance(float angle1, float angle2)
{
  float distance = fmodf(fabsf(angle1 - angle2), 360.0f);

  if (distance > 180.0f) {
    distance = 360.0f - distance;
  }

  return distance;
}

// wrap angle to [0, 360]
__attribute__((unused)) static float wrap_angle(float angle)
{
  if (angle < 0.0f) {
    angle += 360.0f;
  } else if (angle >= 360.0f) {
    angle -= 360.0f;
  }

  return angle;
}

float32_t angle_12, angle_23, angle_31;
int8_t configuration[3] = {0, 0, 0};

float32_t SL_XCORR_GetAngle(int16_t* M1_data, int16_t* M2_data, int16_t* M3_data, size_t dataSize)
{
  int32_t tau, k;
  int32_t delta_t12 = 0, delta_t23 = 0, delta_t31 = 0;
  int64_t MAX12 = 0, MAX23 = 0, MAX31 = 0;

  int64_t correlation = 0;

  float32_t D12[2], D23[2], D31[2];  // Direction in the global coordinate system

  int32_t M12_TAUD = (int32_t)(ACOUSTIC_SL_M12_DISTANCE * (float32_t)AUDIO_IN_SAMPLING_FREQUENCY / 343.1f);
  int32_t M23_TAUD = (int32_t)(ACOUSTIC_SL_M23_DISTANCE * (float32_t)AUDIO_IN_SAMPLING_FREQUENCY / 343.1f);
  int32_t M31_TAUD = (int32_t)(ACOUSTIC_SL_M31_DISTANCE * (float32_t)AUDIO_IN_SAMPLING_FREQUENCY / 343.1f);

  for (tau = -M12_TAUD; tau <= M12_TAUD; tau++) {
    correlation = 0;
    for (k = M12_TAUD; k < dataSize - M12_TAUD; k++) {
      correlation += (int32_t)M2_data[k] * (int32_t)M1_data[k + tau];
    }
    if (correlation > MAX12) {
      MAX12 = correlation;
      delta_t12 = tau;
    }
  }

  for (tau = -M23_TAUD; tau <= M23_TAUD; tau++) {
    correlation = 0;
    for (k = M23_TAUD; k < dataSize - M23_TAUD; k++) {
      correlation += (int32_t)M3_data[k] * (int32_t)M2_data[k + tau];
    }
    if (correlation > MAX23) {
      MAX23 = correlation;
      delta_t23 = tau;
    }
  }

  for (tau = -M31_TAUD; tau <= M31_TAUD; tau++) {
    correlation = 0;
    for (k = M31_TAUD; k < dataSize - M31_TAUD; k++) {
      correlation += (int32_t)M1_data[k] * (int32_t)M3_data[k + tau];
    }
    if (correlation > MAX31) {
      MAX31 = correlation;
      delta_t31 = tau;
    }
  }

  angle_12 = (float32_t)(delta_t12 + M12_TAUD) * 90.0f / (float32_t)M12_TAUD;
  angle_23 = (float32_t)(delta_t23 + M23_TAUD) * 90.0f / (float32_t)M23_TAUD;
  angle_31 = (float32_t)(delta_t31 + M31_TAUD) * 90.0f / (float32_t)M31_TAUD;

  D12[0] = angle_12 + 26.85f;
  D12[1] = 26.85f - angle_12;

  D23[0] = angle_23 + 153.15f;
  D23[1] = 153.15f - angle_23;

  D31[0] = angle_31 - 90;
  D31[1] = -90 - angle_31;

  float MIN_DISAGREEMENT = 10000000;
  float disagreement = 0;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      disagreement = smallest_angular_distance(D12[i], D23[j]);
      if (disagreement < MIN_DISAGREEMENT) {
        MIN_DISAGREEMENT = disagreement;

        configuration[0] = i;
        configuration[1] = j;
      }
    }
  }

  // int8_t select = 0;
  // float MIN_DEVIATION = 1000000000;
  // float deviation = 0;
  // float L[] = {angle_12, angle_23, angle_31};
  // float D[] = {D12[configuration[0]], D23[configuration[1]], D31[configuration[2]]};

  // for (int i = 0; i < 3; i++) {
  //   if (configuration[i] == 0) {
  //     deviation = smallest_angular_distance(L[i], 90.0f);

  //     if (deviation < MIN_DEVIATION) {
  //       MIN_DEVIATION = deviation;
  //       select = i;
  //     }  // int8_t select = 0;
  // float MIN_DEVIATION = 1000000000;
  // float deviation = 0;
  // float L[] = {angle_12, angle_23, angle_31};
  // float D[] = {D12[configuration[0]], D23[configuration[1]], D31[configuration[2]]};

  // for (int i = 0; i < 3; i++) {
  //   if (configuration[i] == 0) {
  //     deviation = smallest_angular_distance(L[i], 90.0f);

  //     if (deviation < MIN_DEVIATION) {
  //       MIN_DEVIATION = deviation;
  //       select = i;
  //     }
  //   }
  // }
  //   }
  // }

  if (configuration[0] == 0 && configuration[1] == 0) {
    return D12[0];
  } else if (configuration[0] == 0 && configuration[1] == 1) {
    return D12[0];
  } else if (configuration[0] == 1 && configuration[1] == 0) {
    return D23[0];
  } else {
    return -1;
  }

  // return wrap_angle(D[select]);
}
