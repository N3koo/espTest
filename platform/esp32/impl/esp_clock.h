#pragma once

#include "interfaces/interfaces.h"
#include "freertos/FreeRTOS.h"

class EspClock final : public IClock {
    public:
        uint32_t millis() const override {
            return xTaskGetTickCount() * portTICK_PERIOD_MS;
        }
};
