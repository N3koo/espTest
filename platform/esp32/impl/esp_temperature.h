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
                .unit_id = ADC_UNIT_1,
                .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
                .ulp_mode = ADC_ULP_MODE_DISABLE
            };

            adc_oneshot_chan_cfg_t chan_config = { 
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT
            };

            adc_cali_line_fitting_config_t cali_config = {
                .unit_id = ADC_UNIT_1,
                .atten = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT,
                .default_vref = 0
            };

            ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle_));
            ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle_, channel_, &chan_config));
            ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle_));
        }

        int16_t readCelsius() const override {
            int raw, mv, sum = 0;
            for (int i = 0; i < SAMPLES; i++) {
                esp_err_t ret = adc_oneshot_read(adc_handle_, channel_, &raw);
                if (ret != ESP_OK) { return SENSOR_FAULT; } // Error reading ADC
                sum += raw;
            }

            esp_err_t err = adc_cali_raw_to_voltage(adc_cali_handle_, sum / SAMPLES, &mv);
            if (err != ESP_OK) { return SENSOR_FAULT; } // Error converting to voltage

            if (mv < V_MIN_MV) { return SENSOR_FAULT; } // Below minimum voltage, return error;
            if (mv > V_MAX_MV) { return SENSOR_FAULT; } // Above maximum voltage, return error;

            int32_t span = V_MAX_MV - V_MIN_MV;
            int32_t temp = T_MIN_C + ((int32_t)(mv - V_MIN_MV) * (T_MAX_C - T_MIN_C) + span / 2) / span;

            return (int16_t)temp;
        }

    private:
        adc_oneshot_unit_handle_t adc_handle_ = nullptr;
        adc_cali_handle_t adc_cali_handle_ = nullptr;
        adc_channel_t channel_;

        static constexpr int32_t V_MIN_MV = 150;  // Minimum voltage in mV
        static constexpr int32_t V_MAX_MV = 3500; // Maximum voltage in mV
        static constexpr int32_t T_MIN_C = 0;    // Minimum temperature in Celsius
        static constexpr int32_t T_MAX_C  = 100; // Maximum temperature in Celsius
        static constexpr int32_t SAMPLES = 16; // Number of samples for averaging
        static constexpr int16_t SENSOR_FAULT = INT16_MIN; // Error value for temperature reading
        static constexpr int32_t MAX_ADC_VALUE = 4095; // Maximum ADC value for 12-bit resolution

        static_assert(SAMPLES * MAX_ADC_VALUE < INT32_MAX, "SAMPLES * MAX_ADC_VALUE must be less than INT32_MAX to avoid overflow");
};
