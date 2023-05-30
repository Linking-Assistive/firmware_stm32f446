#include "message_controller.h"

#include <stdlib.h>
#include <string.h>

#include "sound_localization.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "vibration_motor.h"

#define MSG_SIZE    5  // size + 1(\n)
#define BUFFER_SIZE 20

static uint8_t UART_raw_buffer[BUFFER_SIZE];

volatile uint16_t UART_received_size = 0;

const char* response_L = "LL\n";
const char* response_M = "MM\n";
const char* response_R = "RR\n";

static void sendResponse(uint8_t direction)
{
  switch (direction) {
    case 0:
      HAL_UART_Transmit_DMA(&huart3, response_L, 3);
      break;

    case 1:
      HAL_UART_Transmit_DMA(&huart3, response_M, 3);
      break;

    case 2:
      HAL_UART_Transmit_DMA(&huart3, response_R, 3);
      break;

    default:
      break;
  }
}

static void dataProcess(uint16_t size)
{
  if (size != MSG_SIZE) {
    return;
  }

  int strength = 0;
  int direction = 0;

  switch (UART_raw_buffer[0]) {
    case 'S':
    case 'D':
    case 's':
    case 'd':
      strength = atoi((char*)&UART_raw_buffer[1]);
      direction = SL_get_direction();
      vibration_motor_start(direction, strength);
      sendResponse(direction);
      break;

    default:
      break;
  }
}

void message_controller_init()
{
  HAL_UARTEx_ReceiveToIdle_DMA(&huart3, UART_raw_buffer, BUFFER_SIZE);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t size)
{
  UART_received_size = size;

  dataProcess(size);

  message_controller_init();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
  message_controller_init();
}