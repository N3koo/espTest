#include "impl/gpio_output.h"
#include "freertos/FreeRTOS.h"

static GpioOutput led(GPIO_NUM_2);

extern "C" void app_main() {
    led.init();
    while (true) {
        led.set(true);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        led.set(false);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}