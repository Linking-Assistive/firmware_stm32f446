#include "led.h"

#include "main.h"
#include "stm32f4xx_hal.h"

static uint32_t led_lastTime = 0;

void led_loop(void)
{
  if (HAL_GetTick() - led_lastTime > 500) {
    led_lastTime = HAL_GetTick();

    HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
  }
}