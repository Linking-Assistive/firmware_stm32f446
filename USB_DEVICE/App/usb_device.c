/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usb_device.c
 * @version        : v1.0_Cube
 * @brief          : This file implements the USB Device
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"

#include "usbd_audio_if.h"
#include "usbd_core.h"
#include "usbd_desc.h"

/* USER CODE BEGIN Includes */
#include "audio_sample.h"
#include "usbd_audio_in.h"

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/**
 * @brief  Fills USB audio buffer with the right amount of data, depending on the
 *     channel/frequency configuration
 * @param  audioData: pointer to the PCM audio data
 * @param  PCMSamples: number of PCM samples to be passed to USB engine
 * @note Depending on the calling frequency, a coherent amount of samples must be passed to
 *       the function. E.g.: assuming a Sampling frequency of 16 KHz and 1 channel,
 *       you can pass 16 PCM samples if the function is called each millisecond,
 *       32 samples if called every 2 milliseconds and so on.
 */
void Send_Audio_to_USB(int16_t* audioData, uint16_t PCMSamples)
{
  if (USBD_FAIL == USBD_AUDIO_Data_Transfer(&hUsbDeviceFS, (int16_t*)audioData, PCMSamples)) {
    Error_Handler();
  }
}

/* USER CODE END 1 */

/**
 * Init USB device Library, add supported class and start the library
 * @retval None
 */
void MX_USB_DEVICE_Init(void)
{
  /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */
  USBD_AUDIO_Init_Microphone_Descriptor(&hUsbDeviceFS, AUDIO_IN_SAMPLING_FREQUENCY, 1);

  /* USER CODE END USB_DEVICE_Init_PreTreatment */

  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_AUDIO) != USBD_OK) {
    Error_Handler();
  }
  if (USBD_AUDIO_RegisterInterface(&hUsbDeviceFS, &USBD_AUDIO_fops_FS) != USBD_OK) {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
    Error_Handler();
  }

  /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */

  /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
 * @}
 */

/**
 * @}
 */
