#include "SccbScanner.hpp"

// ============================================================
// SccbScanner
// ============================================================
SccbScanner::SccbScanner(int sda, int scl)
    : _sda(sda), _scl(scl)
{
}

// close the SCCB bus on the camera's
void SccbScanner::begin()
{
    // 100 kHz
    Wire.setClock(100000);
    Wire.begin(_sda, _scl);
}

// close the SCCB bus on the camera's
void SccbScanner::end()
{
    Wire.end();
}

///Probes all 7-bit addresses (1–127), prints every device that ACKs, returns OV5640_SCCB_ADDR if the OV5640 is present.
uint8_t SccbScanner::scan()
{
    uint8_t found = 0;

    Serial.println();
    Serial.println("==============================================");
    Serial.println("                SCCB SCAN");
    Serial.println("==============================================");

    for (uint8_t addr = 1; addr < 128; addr++)
    {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();

        if (err == 0)
        {
            Serial.printf("Device found at 0x%02X\n", addr);

            if (addr == OV5640_SCCB_ADDR)
                found = addr;
        }
    }

    if (found == 0)
        Serial.println("No SCCB devices found.");

    Serial.println("==============================================");

    return found;
}

bool SccbScanner::readReg(uint8_t addr, uint16_t reg, uint8_t &value)
{
    Wire.beginTransmission(addr);
    Wire.write((uint8_t)(reg >> 8));
    Wire.write((uint8_t)(reg & 0xFF));

    // Repeated start, then read a single byte.
    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(addr, (uint8_t)1) != 1)
        return false;

    value = Wire.read();
    return true;
}

///readSensorId(addr=OV5640_SCCB_ADDR) — reads the OV5640 ID registers (0x300A/0x300B → PID, 0x300C/0x300D → MID) over SCCB 
///and prints them; returns true if PID is 0x5640.
bool SccbScanner::readSensorId(uint8_t addr)
{
    uint8_t pidH = 0, pidL = 0, midH = 0, midL = 0;

    bool ok = true;
    ok &= readReg(addr, 0x300A, pidH); // PID high
    ok &= readReg(addr, 0x300B, pidL); // PID low
    ok &= readReg(addr, 0x300C, midH); // MID high
    ok &= readReg(addr, 0x300D, midL); // MID low

    Serial.println();
    Serial.println("----- SENSOR ID -----");

    if (!ok)
    {
        Serial.println("ERROR: failed to read sensor ID registers.");
        return false;
    }

    uint16_t pid = ((uint16_t)pidH << 8) | pidL;

    Serial.printf("Sensor address  : 0x%02X\n", addr);
    Serial.printf("Sensor PID      : 0x%04X\n", pid);
    Serial.printf("Sensor MIDH     : 0x%02X\n", midH);
    Serial.printf("Sensor MIDL     : 0x%02X\n", midL);

    Serial.println("===================");

    return (pid == 0x5640);
}

void SccbScanner::run()
{
    //STEP 1: Open the SCCB bus
    begin();

    uint8_t addr = scan();

    if (addr == OV5640_SCCB_ADDR)
    {
        readSensorId(addr);
    }
    else
    {
        Serial.printf("OV5640 (0x%02X) not detected on SCCB bus.\n", OV5640_SCCB_ADDR);
    }

    end();
}
