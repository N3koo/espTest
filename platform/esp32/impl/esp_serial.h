#pragma once

#include "interfaces/interfaces.h"
#include "driver/uart.h"

#include <cstring>

class EspSerial final : public ISerial {
    public:
        explicit EspSerial(uart_port_t uartPort, int32_t baudRate, int8_t txPin, int8_t rxPin) :
            uartPort_(uartPort),
            baudRate_(baudRate), 
            txPin_(txPin),
            rxPin_(rxPin) {}

        void init() {
            uart_config_t uart_config = {};
            uart_config.baud_rate = baudRate_;
            uart_config.data_bits = UART_DATA_8_BITS;
            uart_config.parity = UART_PARITY_DISABLE;
            uart_config.stop_bits = UART_STOP_BITS_1;
            uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
            uart_config.source_clk = UART_SCLK_APB;

            ESP_ERROR_CHECK(uart_driver_install(uartPort_, BUFFER_SIZE, BUFFER_SIZE, 0, nullptr, 0));
            ESP_ERROR_CHECK(uart_param_config(uartPort_, &uart_config));
            ESP_ERROR_CHECK(uart_set_pin(uartPort_, txPin_, rxPin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        }

        void write(const char* data) override {
            uart_write_bytes(uartPort_, data, strlen(data));
        }

        void readLine(char* buffer, uint16_t length) override {
            if (length == 0) { return; }
            if (length > MAX_LINE_LENGTH) { length = MAX_LINE_LENGTH;}
            
            int bytesRead = uart_read_bytes(uartPort_, reinterpret_cast<uint8_t*>(buffer), length - 1, portMAX_DELAY);
            if (bytesRead < 0) { bytesRead = 0; } // Handle error case

            buffer[bytesRead] = '\0'; // Null-terminate the string
        }

    private:
        uart_port_t uartPort_;
        int32_t baudRate_;
        int8_t txPin_;
        int8_t rxPin_;

        static constexpr uint16_t MAX_LINE_LENGTH = 256;
        static constexpr uint16_t BUFFER_SIZE = MAX_LINE_LENGTH * 2;
};
