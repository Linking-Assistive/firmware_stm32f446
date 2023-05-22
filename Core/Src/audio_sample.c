#include "audio_sample.h"

// #include "acoustic_sl.h"
#include "i2s.h"

uint16_t dataBuffer[4];
uint16_t dataBuffer_1[4];
int16_t LR_data[3];

// static AcousticSL_Handler_t libSoundSourceLoc_Handler_Instance;

void audioSampleStartEverySample_IT()
{
  HAL_I2S_Receive_IT(&hi2s2, dataBuffer, 4);
  // HAL_I2S_Receive_IT(&hi2s3, dataBuffer_1, 4);
}
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef* hi2s)
{
  if (hi2s->Instance == SPI2) {
    LR_data[0] = (int16_t)dataBuffer[0];
    LR_data[1] = (int16_t)dataBuffer[2];
    HAL_I2S_Receive_IT(&hi2s2, dataBuffer, 4);
  }
  if (hi2s->Instance == SPI3) {
    LR_data[2] = (int16_t)dataBuffer_1[0];
    HAL_I2S_Receive_IT(&hi2s3, dataBuffer_1, 4);
  }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef* hi2s)
{
  if (hi2s->Instance == SPI2) {
    HAL_I2S_Receive_IT(&hi2s2, dataBuffer, 4);
  } else if (hi2s->Instance == SPI3) {
    HAL_I2S_Receive_IT(&hi2s3, dataBuffer_1, 4);
  }
}
