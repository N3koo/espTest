#pragma once

#include "interfaces/interfaces.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "driver/gpio.h"

#define V_MIN 150
#define V_MAX 3500
#define T_MIN 0
#define T_MAX 1000

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

            adc_cali_handle_t cali_handle = {
                .unit_id = ADC_UNIT_1,
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT
            };

            ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle_));
            ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle_, channel_, &chan_config));
            ESP_ERROR_CHECK(adc_cali_create_scheme(&cali_handle, &adc_cali_handle_));
        }

        int16_t readCelsius() const override {
            int raw, mv, sum = 0;
            for (int i = 0; i < 16; i++) {
                esp_err_t ret = adc_oneshot_read(adc_handle_, channel_, &raw);
                if (ret != ESP_OK) { return -1; } // Error reading ADC
                sum += raw;
            }

            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle_, sum / 16, &mv));
            if (mv < V_MIN) mv = V_MIN;
            if (mv > V_MAX) mv = V_MAX;

            int32_t span = V_MAX - V_MIN;
            int32_t temp = T_MIN + ((int32_t)(mv - V_MIN) * (T_MAX - T_MIN) + span / 2) / span;

            return (int16_t)temp;
        }

    private:
        adc_oneshot_unit_handle_t adc_handle_;
        adc_cali_handle_t adc_cali_handle_;
        adc_channel_t channel_;
};