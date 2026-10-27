#pragma once

#include "interfaces/interfaces.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"

#if CONFIG_FREERTOS_HZ != 1000
    #error "CONFIG_FREERTOS_HZ must be set to 1000 in sdkconfig"
#endif

class EspClock final : public IClock {
    public:
        uint32_t millis() const override {
            return xTaskGetTickCount() * portTICK_PERIOD_MS;
        }
};
