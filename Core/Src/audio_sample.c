#include "audio_sample.h"

#include "i2s.h"
#include "main.h"
#include "sound_localization.h"

// I2S 32bit frame, 64bits(4 half-words) per sample (L/R channel)
static uint16_t rawDataInBuffer_LR[AUDIO_SAMPLE_PER_PACKET * 4 * 2] = {0};  // * 2(double buffer)
static uint16_t rawDataInBuffer_M[AUDIO_SAMPLE_PER_PACKET * 4 * 2] = {0};   // * 2(double buffer)
static uint32_t is_otherReceived = 0;

static int16_t aPCMBufferIN[AUDIO_IN_CHANNELS * AUDIO_SAMPLE_PER_PACKET] = {0};
int16_t LR_data[2];

static void dataProcess(uint16_t* src, int16_t* dst)
{
  int16_t* tmp_ptr = dst;
  for (int i = 0; i < AUDIO_SAMPLE_PER_PACKET; i++, tmp_ptr += 2, src += 4) {
    *tmp_ptr = (int16_t)(*src);
    *(tmp_ptr + 1) = (int16_t)(*(src + 2));
  }
}

static void mergeData(uint16_t* buffer_LF, uint16_t* buffer_M, int16_t* dst)
{
  for (int i = 0; i < AUDIO_SAMPLE_PER_PACKET; i++, dst += 2, buffer_LF += 4, buffer_M += 4) {
    *dst = (int16_t)(*buffer_LF);
    *(dst + 1) = (int16_t)(*buffer_M);
  }
}

void audioStartRecord_DMA()
{
  HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)rawDataInBuffer_LR,
                      AUDIO_SAMPLE_PER_PACKET * 2 * 2);  // *2(L/R Channel) * 2 (2ms double buffer)
  HAL_I2S_Receive_DMA(&hi2s3, (uint16_t*)rawDataInBuffer_M,
                      AUDIO_SAMPLE_PER_PACKET * 2 * 2);  // *2(L/R Channel) * 2 (2ms double buffer)
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef* hi2s)
{
  if (is_otherReceived == 0) {
    is_otherReceived = 1;
  } else {
    is_otherReceived = 0;
    mergeData(rawDataInBuffer_LR, rawDataInBuffer_M, aPCMBufferIN);

    for (int i = 0; i < AUDIO_IN_MS_PER_PACKET; i++) {
      SL_DataInput(&aPCMBufferIN[i * AUDIO_IN_SAMPLE_PER_MS]);
    }
    LR_data[0] = aPCMBufferIN[4];
    LR_data[1] = aPCMBufferIN[5];
  }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef* hi2s)
{
  if (is_otherReceived == 0) {
    is_otherReceived = 1;
  } else {
    is_otherReceived = 0;
    mergeData(&rawDataInBuffer_LR[AUDIO_SAMPLE_PER_PACKET * 4], &rawDataInBuffer_M[AUDIO_SAMPLE_PER_PACKET * 4],
              aPCMBufferIN);

    for (int i = 0; i < AUDIO_IN_MS_PER_PACKET; i++) {
      SL_DataInput(&aPCMBufferIN[i * AUDIO_IN_SAMPLE_PER_MS]);
    }
    LR_data[0] = aPCMBufferIN[4];
    LR_data[1] = aPCMBufferIN[5];
  }
}
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef* hi2s)
{
  Error_Handler();
}
