#include "ResolutionProbe.hpp"

// OV5640 output-size (timing) registers
// DVP Output Resolution Registers
static const uint16_t REG_HTS_H = 0x3808; // horizontal width
static const uint16_t REG_HTS_L = 0x3809;
static const uint16_t REG_VTS_H = 0x380A; // vertical height
static const uint16_t REG_VTS_L = 0x380B;
// Software reset register
static const uint16_t REG_SYS_RESET = 0x3008; // Bit[7]: Software reset

ResolutionProbe::ResolutionProbe(int sda, int scl)
    : _sda(sda), _scl(scl)
{
}

// 1) XCLK - same as SccbScanner (1-bit resolution => 50% 20MHz square wave)
void ResolutionProbe::startXclk()
{
    ledcSetup(kXclkChannel, kXclkFreqHz, 1);
    ledcAttachPin(XCLK_GPIO_NUM, kXclkChannel);
    ledcWrite(kXclkChannel, 1);
}

void ResolutionProbe::stopXclk()
{
    ledcWrite(kXclkChannel, 0);
    ledcDetachPin(XCLK_GPIO_NUM);
}

// 2) init SCCB communication (raw Wire / I2C peripheral)
void ResolutionProbe::initSccb()
{
    Wire.begin(_sda, _scl);
    Wire.setClock(100000);
}

// 3) cool down - give the OV5640 time to boot once XCLK is present
void ResolutionProbe::coolDown()
{
    delay(300);
}

// raw 8-bit register write (SCCB 2-phase: address write + STOP, data write)
bool ResolutionProbe::writeReg(uint16_t reg, uint8_t value)
{
    Wire.beginTransmission(OV5640_SCCB_ADDR);
    Wire.write((uint8_t)(reg >> 8));
    Wire.write((uint8_t)(reg & 0xFF));
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

// raw 8-bit register read (SCCB 2-phase with retry)
bool ResolutionProbe::readReg(uint16_t reg, uint8_t &value)
{
    for (int i = 0; i < 3; i++)
    {
        Wire.beginTransmission(OV5640_SCCB_ADDR);
        Wire.write((uint8_t)(reg >> 8));
        Wire.write((uint8_t)(reg & 0xFF));
        if (Wire.endTransmission() != 0)
            continue;
        delayMicroseconds(10);
        if (Wire.requestFrom(OV5640_SCCB_ADDR, (uint8_t)1) != 1)
            continue;
        value = Wire.read();
        return true;
    }
    return false;
}

// 4) read resolution directly from registers and print
void ResolutionProbe::readResolution()
{
    uint8_t hH = 0, hL = 0, vH = 0, vL = 0;
    readReg(REG_HTS_H, hH);
    readReg(REG_HTS_L, hL);
    readReg(REG_VTS_H, vH);
    readReg(REG_VTS_L, vL);

    uint16_t width = ((uint16_t)(hH & 0x0F) << 8) | hL;
    uint16_t height = ((uint16_t)(vH & 0x0F) << 8) | vL;

    Serial.printf("  REG 0x3808 (HTS_H) = 0x%02X\n", hH);
    Serial.printf("  REG 0x3809 (HTS_L) = 0x%02X\n", hL);
    Serial.printf("  REG 0x380A (VTS_H) = 0x%02X\n", vH);
    Serial.printf("  REG 0x380B (VTS_L) = 0x%02X\n", vL);
    Serial.printf("  => Resolution      : %u x %u\n", width, height);
}

// 5) write resolution value directly to registers, then read back
void ResolutionProbe::writeResolution(uint16_t width, uint16_t height)
{
    uint8_t hH = (width >> 8) & 0x0F;
    uint8_t hL = width & 0xFF;
    uint8_t vH = (height >> 8) & 0x0F;
    uint8_t vL = height & 0xFF;

    writeReg(REG_HTS_H, hH);
    writeReg(REG_HTS_L, hL);
    writeReg(REG_VTS_H, vH);
    writeReg(REG_VTS_L, vL);

    Serial.printf("  Wrote %u x %u to 0x3808..0x380B\n", width, height);
    Serial.println("  (Only these size registers change; full timing/scaling");
    Serial.println("   is configured by esp_camera. Reading back below:)");
    readResolution();
}

// 6) restart the camera (software reset) then read current resolution
void ResolutionProbe::restartCamera()
{
    Serial.println("  Triggering software reset (REG 0x3008 = 0x80)...");
    writeReg(REG_SYS_RESET, 0x80);
    delay(50);
    readResolution();
}

void ResolutionProbe::run()
{
    Serial.println("==============================================");
    Serial.println("         OV5640 RESOLUTION PROBE");
    Serial.println("==============================================");

    // 1) XCLK
    startXclk();
    // 2) SCCB
    initSccb();
    // 3) cool down
    coolDown();

    // 4) read resolution directly from registers
    Serial.println();
    Serial.println("[1] Read resolution directly from registers:");
    readResolution();

    // 5) write a resolution to registers, then read later after writing
    Serial.println();
    Serial.println("[2] Write 320x240 to registers, then read back:");
    writeResolution(320, 240);

    // 6) restart camera, then read current resolution
    Serial.println();
    Serial.println("[3] Restart camera (software reset), then read resolution:");
    restartCamera();

    stopXclk();
    Serial.println();
    Serial.println("==============================================");
    Serial.println("           RESOLUTION PROBE DONE");
    Serial.println("==============================================");
}
