#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "PinsConfig.hpp"

// ============================================================
// ResolutionProbe
// Standalone, low-level OV5640 register playground. It mirrors
// SccbScanner's XCLK + raw Wire (SCCB) approach but exercises the
// sensor's output-size registers (0x3808..0x380B) directly:
//   1) start XCLK (same as SccbScanner)
//   2) init SCCB communication (Wire)
//   3) cool down (let the sensor boot)
//   4) read resolution directly from registers and print
//   5) write a resolution to the registers, then read it back
//   6) restart the camera (software reset) and re-read resolution
// It does NOT use the esp_camera driver.
// ============================================================

class ResolutionProbe
{
public:
    ResolutionProbe(int sda = SIOD_GPIO_NUM, int scl = SIOC_GPIO_NUM);
    void run();

private:
    void startXclk();
    void stopXclk();
    void initSccb();
    void coolDown();

    bool writeReg(uint16_t reg, uint8_t value);
    bool readReg(uint16_t reg, uint8_t &value);

    void readResolution();
    void writeResolution(uint16_t width, uint16_t height);
    void restartCamera();

    int _sda;
    int _scl;

    static const uint8_t  kXclkChannel = 1; // keep clear of camera channel 0
    static const uint32_t kXclkFreqHz  = 20000000;
};
