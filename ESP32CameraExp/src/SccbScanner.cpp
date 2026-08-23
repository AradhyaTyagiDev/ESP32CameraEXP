#include "SccbScanner.hpp"

// ============================================================
// SccbScanner
// ============================================================

// XCLK (EXTCLK) settings. The OV5640 SCCB bus is only alive once this
// master clock is running; we drive it here so a pre-init scan can
// actually reach the sensor (the camera driver normally starts it).
namespace
{
    const uint8_t kXclkChannel = 1; // keep clear of camera's channel 0
    const uint32_t kXclkFreqHz = 20000000;
}

// Constructor:
// Stores the SDA and SCL pins that will be used for the camera SCCB bus.
// These pins are coming from the camera config in PinsConfig.hpp.
SccbScanner::SccbScanner(int sda, int scl)
    : _sda(sda), _scl(scl)
{
}

// startXclk()
// Drive the OV5640 master clock (XCLK/EXTCLK) via LEDC so the sensor's
// SCCB interface becomes responsive. Without this, a pre-init scan finds
// nothing because the sensor is not clocked.
void SccbScanner::startXclk()
{
    // 1-bit resolution: period = 2 ticks, duty 1 => 50% square wave.
    // (20 MHz is not achievable with higher duty resolution; this matches
    // how esp_camera's own XCLK setup drives the sensor.)
    ledcSetup(kXclkChannel, kXclkFreqHz, 1);
    ledcAttachPin(XCLK_GPIO_NUM, kXclkChannel);
    // After this function returns, the ESP32 is continuously generating the camera clock in hardware.
    // The CPU does not need to manually toggle the pin
    ledcWrite(kXclkChannel, 1);
}

// stopXclk()
// Stop the clock we started and release the pin.
void SccbScanner::stopXclk()
{
    ledcWrite(kXclkChannel, 0);
    ledcDetachPin(XCLK_GPIO_NUM);
}

// begin()
// Starts the SCCB bus using the ESP32's Wire object.
// SCCB is basically I2C with a camera sensor on it.
// We set the clock to 100 kHz because OV5640 usually works fine at this speed.
void SccbScanner::begin()
{
    // That ordering matters: the camera clock starts before Wire attempts to scan the sensor.
    startXclk();            // clock the sensor FIRST
    Wire.begin(_sda, _scl); // initialize bus on camera SDA/SCL pins
    // We can change the clock speed:Wire.setClock(400000);  // 400 kHz
    // The OV5640 SCCB interface generally supports speeds up to approximately 400 kHz
    // For an OV5640, 100 kHz is the conservative and commonly used speed. It is usually the best starting point because it provides more timing margin, especially with long wires, weak pull-up resistors, breadboards, or a noisy power supply.
    /* The scan will complete faster and register reads will take less time. However, higher speed is more sensitive to:
        SDA/SCL pull-up resistor values
        Wire length
        Electrical noise
        Camera-module design
        Voltage-level compatibility
        Bus capacitance

        Wire.setClock(100000);  // Most reliable starting point
        Wire.setClock(200000);  // Intermediate test
        Wire.setClock(400000);  // Usual upper practical speed
        Wire.setClock(600000);  // Experimental; not recommended initially
     */
    // 600 kHz is above the usual 400 kHz Fast-mode limit, so communication may become unreliable.
    // Practically, it fails above 950000 Hz, so we can set it to 950000 Hz for a faster scan while still being within the sensor's capabilities.
    Wire.setClock(950000); // 100 kHz bus speed (after begin)

    Serial.printf("Final SCCB clock: %lu Hz\n", Wire.getClock());
}

// end()
// Stops the SCCB bus when we are done.
// This is important to release the I2C peripheral.
// end() calls it after SCCB communication is finished.
void SccbScanner::end()
{
    Wire.end();
    stopXclk(); // stop XCLK and release the pin
}

// scan()
// Purpose:
//   Walk through all possible 7-bit SCCB addresses (1..127),
//   and check which device responds with an ACK.
// Practical meaning:
//   We are asking "Is there any camera or sensor connected at this address?"
//
// Returns:
//   The address of the OV5640 if it is found, otherwise 0.
uint8_t SccbScanner::scan()
{
    uint8_t found = 0;

    Serial.println();
    Serial.println("==============================================");
    Serial.println("                SCCB SCAN");
    Serial.println("==============================================");

    // Test every possible address from 1 to 127.
    // Each address is a 7-bit I2C/SCCB address.
    for (uint8_t addr = 1; addr < 128; addr++)
    {
        // Start communication with a candidate device.
        Wire.beginTransmission(addr);

        // endTransmission() returns 0 if the device ACKed the address.
        // If no device is there, it returns an error code instead.
        uint8_t err = Wire.endTransmission();

        // err == 0 means a device responded.
        if (err == 0)
        {
            Serial.printf("Device found at 0x%02X\n", addr);

            // If we found the OV5640 camera sensor, save its address.
            if (addr == OV5640_SCCB_ADDR)
                found = addr;
        }
    }

    // If the sensor was not found, print a message.
    if (found == 0)
        Serial.println("No SCCB devices found.");

    Serial.println("==============================================");

    return found;
}

// readReg()
// Purpose:
//   Read a single byte from one register inside the camera.
// Practical meaning:
//   "Talk to camera at address X, tell it register Y, then read back the byte stored there."
//
// Parameters:
//   addr = camera device address on the SCCB bus
//   reg  = register address inside the camera (16-bit register number)
//   value = output variable where the returned byte is stored
//
// Example:
//   readReg(0x3C, 0x300A, pidH);
//   -> means: read register 0x300A from camera 0x3C and store it in pidH
bool SccbScanner::readReg(uint8_t addr, uint16_t reg, uint8_t &value)
{
    // 1) Tell the sensor which device we want to talk to.
    // Meaning: I am starting a transaction with device at address 'addr'.
    Wire.beginTransmission(addr);

    // 2) Send the register address in 16-bit form.
    // For a register like 0x300A:
    //   high byte = 0x30
    //   low byte  = 0x0A
    // This is MSB-first, because the camera expects register addresses as 2 bytes.
    Wire.write((uint8_t)(reg >> 8));   // upper byte of the register
    Wire.write((uint8_t)(reg & 0xFF)); // lower byte of the register

    // 3) Send the register address and keep the bus active for a repeated start.
    // endTransmission(false) means "do not send STOP yet" because we want to read data next.
    // endTransmission(false) = “okay, now keep bus open and prepare for the answer”
    // Wire.endTransmission(); // default = true, sends STOP immediately. 
    //          If you use the default true, it stops the bus before the read operation, so the register-read sequence breaks.
    if (Wire.endTransmission(false) != 0)
        return false;

    // 4) Ask the camera to send 1 byte of data back.
    // This is the actual read phase of the transaction.
    if (Wire.requestFrom(addr, (uint8_t)1) != 1)
        return false;

    // 5) Read the single byte returned by the camera.
    value = Wire.read();
    return true;
}

// readSensorId()
// Purpose:
//   Read the OV5640 identification registers to confirm the chip is really the expected sensor.
// Practical meaning:
//   The camera exposes its product ID and manufacturer ID in specific registers.
//   We read them and verify the chip matches the OV5640.
//
// Registers:
//   0x300A / 0x300B = Product ID (PID)
//   0x300C / 0x300D = Manufacturer ID (MID)
bool SccbScanner::readSensorId(uint8_t addr)
{
    // These variables hold the individual bytes read from the sensor.
    uint8_t pidH = 0, pidL = 0, midH = 0, midL = 0;

    // Read all 4 register values.
    // Each readReg call reads one byte from one register.
    bool ok = true;
    ok &= readReg(addr, 0x300A, pidH); // PID high byte
    ok &= readReg(addr, 0x300B, pidL); // PID low byte
    ok &= readReg(addr, 0x300C, midH); // MID high byte
    ok &= readReg(addr, 0x300D, midL); // MID low byte

    Serial.println();
    Serial.println("----- SENSOR ID -----");

    // If any register read failed, communication is bad or the camera is not responding correctly.
    if (!ok)
    {
        Serial.println("ERROR: failed to read sensor ID registers.");
        return false;
    }

    // Combine the two PID bytes into a 16-bit value.
    // Example:
    //   pidH = 0x56
    //   pidL = 0x40
    //   pid  = 0x5640
    uint16_t pid = ((uint16_t)pidH << 8) | pidL;

    // Print the values so we can visually verify the chip identity.
    Serial.printf("Sensor address  : 0x%02X\n", addr); // 0x3C
    Serial.printf("Sensor PID      : 0x%04X\n", pid);  // 0x5640
    Serial.printf("Sensor MIDH     : 0x%02X\n", midH); // 0x22
    Serial.printf("Sensor MIDL     : 0x%02X\n", midL); // 0x00

    Serial.println("===================");

    // OV5640 PID is 0x5640. If the sensor matches, return true.
    return (pid == 0x5640);
}

// run()
// Purpose:
//   Full sequence:
//   1) start SCCB bus
//   2) scan addresses to locate the sensor
//   3) if OV5640 is found, read its ID
//   4) close bus
//
// This is a practical “self-check” before trying to initialize the camera.
void SccbScanner::run()
{
    // STEP 1: Open the SCCB bus on the camera pins.
    begin();

    // STEP 2: Scan the bus and find the camera address.
    uint8_t addr = scan();

    // STEP 3: If the OV5640 is present, verify its identity.
    if (addr == OV5640_SCCB_ADDR)
    {
        readSensorId(addr);
    }
    else
    {
        Serial.printf("OV5640 (0x%02X) not detected on SCCB bus.\n", OV5640_SCCB_ADDR);
    }

    // STEP 4: Close the bus.
    end();
}
