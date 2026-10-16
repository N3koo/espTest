#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void app_main() {

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2), // Configure GPIO2
        .mode = GPIO_MODE_OUTPUT,              // Set as output mode
        .pull_up_en = GPIO_PULLUP_DISABLE,     // Disable pull-up
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Disable pull-down
        .intr_type = GPIO_INTR_DISABLE         // Disable interrupts
    };

    gpio_config(&io_conf); // Apply the configuration

    while(true) {
        printf("Hello, World!\n");
        gpio_set_level(GPIO_NUM_2, 1); // Set GPIO2 high
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
        gpio_set_level(GPIO_NUM_2, 0); // Set GPIO2 low
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}