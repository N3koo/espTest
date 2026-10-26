#pragma once

#include "interfaces/interfaces.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

class EspTemperature final : public ITemperatureSensor {
    public:
        explicit EspTemperature(adc_channel_t channel) : channel_(channel) {}

        void init() {
            adc_oneshot_unit_init_cfg_t init_config = { 
                .unit_id = ADC_UNIT_1 
            };

            adc_oneshot_chan_cfg_t chan_config = { 
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT
            };

            adc_cali_line_fitting_config_t cali_handle = {
                .unit_id = ADC_UNIT_1,
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT
            };

            ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle_));
            ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle_, channel_, &chan_config));
            ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_handle, &adc_cali_handle_));
        }

        int16_t readCelsius() const override {
            int raw, mv, sum = 0;
            for (int i = 0; i < SAMPLES; i++) {
                esp_err_t ret = adc_oneshot_read(adc_handle_, channel_, &raw);
                if (ret != ESP_OK) { return ERROR_VALUE; } // Error reading ADC
                sum += raw;
            }

            esp_err_t err = adc_cali_raw_to_voltage(adc_cali_handle_, sum / SAMPLES, &mv);
            if (err != ESP_OK) { return ERROR_VALUE; } // Error converting to voltage

            if (mv < V_MIN) { return INT16_MIN; } // Below minimum voltage, return error;
            if (mv > V_MAX) { return INT16_MAX; } // Above maximum voltage, return error;

            int32_t span = V_MAX - V_MIN;
            int32_t temp = T_MIN + ((int32_t)(mv - V_MIN) * (T_MAX - T_MIN) + span / 2) / span;

            return (int16_t)temp;
        }

    private:
        adc_oneshot_unit_handle_t adc_handle_ = nullptr;
        adc_cali_handle_t adc_cali_handle_ = nullptr;
        adc_channel_t channel_;

        static constexpr int32_t V_MIN = 150;  // Minimum voltage in mV
        static constexpr int32_t V_MAX = 3500; // Maximum voltage in mV
        static constexpr int32_t T_MIN = 0;    // Minimum temperature in Celsius
        static constexpr int32_t T_MAX = 1000; // Maximum temperature in Celsius
        static constexpr int32_t SAMPLES = 16; // Number of samples for averaging
        static constexpr int16_t ERROR_VALUE = -1; // Error value for temperature reading 
};
