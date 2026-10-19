#pragma once

#include "interfaces/interfaces.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

class EspTemperature final : public ITemperatureSensor {
    public:
        explicit EspTemperature(gpio_num_t pin) {}

        void init() {
            adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
            adc_oneshot_new_unit(&init_config, &adc_handle_);

            adc_oneshot_chan_cfg_t chan_config = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
            adc_oneshot_config_channel(adc_handle_, ADC_CHANNEL_6, &chan_config);
        }

        int16_t readCelsius() const override {
            int adc_reading = 0;
            adc_oneshot_read(adc_handle_, ADC_CHANNEL_6, &adc_reading);
            // Placeholder for actual temperature reading logic
            // In a real implementation, you would read from the sensor connected to the specified pin
            return 25; // Return a dummy temperature value for now
        }

    private:
        adc_oneshot_unit_handle_t adc_handle_;
        gpio_num_t pin_;
};