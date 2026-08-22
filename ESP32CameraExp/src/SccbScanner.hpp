#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "PinsConfig.hpp"

// ============================================================
// SccbScanner
// Scans the SCCB (I2C) bus for the OV5640 at OV5640_SCCB_ADDR, then reads
// the sensor identification registers. Useful as a pre-flight
// check before esp_camera_init() claims the bus.
// ============================================================
class SccbScanner
{
public:
    // Defaults to the camera's SCCB pins (SIOD/SIOC).
    SccbScanner(int sda = SIOD_GPIO_NUM, int scl = SIOC_GPIO_NUM);

    // Scan, then read the sensor ID if OV5640_SCCB_ADDR is detected.
    // Self-contained: opens and closes the bus.
    void run();

private:
    // Start/stop the OV5640 master clock (XCLK) so the SCCB bus is alive
    // during a pre-init scan.
    void startXclk();
    void stopXclk();

    // Read a single 8-bit register over SCCB (16-bit address, MSB first).
    bool readReg(uint8_t addr, uint16_t reg, uint8_t &value);

    int _sda;
    int _scl;

    // Begin the I2C/SCCB bus on the configured pins.
    void begin();

    // Release the I2C peripheral.
    void end();

    // Scan all 7-bit addresses and print every device that ACKs.
    // Returns the OV5640 address (OV5640_SCCB_ADDR) if present, otherwise 0.
    uint8_t scan();

    // Read the OV5640 identification registers and print the result.
    // Returns true if the sensor was found and its ID read OK.
    bool readSensorId(uint8_t addr = OV5640_SCCB_ADDR);
};
