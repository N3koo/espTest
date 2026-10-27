#pragma once
#include <cstdint>

class IOutput {
    public:
        virtual void set(bool on) = 0;
        virtual bool isOn() const = 0;

    protected:
        ~IOutput() = default;
};

class ITemperatureSensor {
    public:
        virtual int16_t readCelsius() const = 0;

    protected:
        ~ITemperatureSensor() = default;
};

class IDoorSensor {
    public:
        virtual bool isOpen() const = 0;

    protected:
        ~IDoorSensor() = default;
};

class IClock {
    public:
        virtual uint32_t millis() const = 0;

    protected:
        ~IClock() = default;
};

class ISerial {
    public:
        virtual void write(const char* data) = 0;
        virtual void readLine(char* buffer, uint16_t length) = 0;

    protected:
        ~ISerial() = default;
};
