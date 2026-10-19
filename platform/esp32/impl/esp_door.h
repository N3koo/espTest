#pragma once

#include "interfaces/interfaces.h"
#include "driver/gpio.h"

class EspDoor final : public IDoorSensor {
    public:
        explicit EspDoor(gpio_num_t pin) : pin_(pin){};

        void init() {
            gpio_reset_pin(pin_);
            gpio_set_direction(pin_, GPIO_MODE_INPUT);
        }

        bool isOpen() const override {
            return gpio_get_level(pin_) == 1;
        };

    private:
        gpio_num_t pin_;
};