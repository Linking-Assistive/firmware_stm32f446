#include "message_controller.h"

#include "stm32f4xx_hal.h"
#include "usart.h"

static uint8_t UART_message_buffer[20];
volatile uint16_t UART_received_size = 0;

void message_controller_init()
{
  HAL_UARTEx_ReceiveToIdle_DMA(&huart3, UART_message_buffer, 20);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t size)
{
  UART_received_size = size;
  message_controller_init();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
  message_controller_init();
}