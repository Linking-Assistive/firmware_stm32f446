#ifndef __AUDIO_SAMPLE_H__
#define __AUDIO_SAMPLE_H__

#define AUDIO_IN_SAMPLING_FREQUENCY 16000
#define AUDIO_IN_CHANNELS           2
#define AUDIO_IN_SAMPLE_PER_MS      (AUDIO_IN_SAMPLING_FREQUENCY / 1000)  // 16
#define AUDIO_IN_MS_PER_PACKET      16
#define AUDIO_SAMPLE_PER_PACKET     (AUDIO_IN_SAMPLE_PER_MS * AUDIO_IN_MS_PER_PACKET)

void audioStartRecord_DMA(void);

#endif