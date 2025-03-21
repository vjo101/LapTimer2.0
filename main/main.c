
#include "driver/gpio.h"
#include <stdio.h>

void app_main(void) {
  const int gpio_range = 38;
  for (int i = 0; i < gpio_range; i++) {
    if (gpio_set_direction(i, GPIO_MODE_INPUT) == ESP_OK) {
      gpio_set_pull_mode(i, GPIO_FLOATING);
      printf("setup GPIO %d", i);
    }
  }

  while (1) {
    for (int i = 0; i < gpio_range; i++) {
      if (gpio_get_level(i) == 1) {
        printf("GPIO %d is high", i);
      }
    }
  }
}