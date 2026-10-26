#pragma once

#include "interfaces/interfaces.h"
#include "driver/gpio.h"

class EspOutput final : public IOutput {
    public:
        explicit EspOutput(gpio_num_t pin) : pin_(pin) {}

        void init() {
            gpio_reset_pin(pin_);
            gpio_set_direction(pin_, GPIO_MODE_OUTPUT);
            set(false);
        }

        void set(bool on) override {
            gpio_set_level(pin_, on ? 1 : 0);
            on_ = on;
        }

        bool isOn() const override {
            return on_;
        }

    private:
        gpio_num_t pin_;
        bool on_ = false;
};