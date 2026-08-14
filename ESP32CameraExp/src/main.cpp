#include <Arduino.h>
#include "AppConfig.hpp"
#include "CameraApp.hpp"
#include "SccbScanner.hpp"

// ============================================================
// Arduino setup
// ============================================================
void setup()
{
    Serial.begin(115200);
    delay(BOOT_SERIAL_DELAY_MS);

    printMCUInfo();

    // Pre-flight: scan SCCB bus and read the sensor ID (works even
    // before esp_camera_init() claims the bus).
    SccbScanner().run();

#if APP_MODE == MODE_BOARD_INFO
    // Board info only: no camera needed.

#else
    if (!initCamera())
    {
        halt("CAMERA INIT FAILED.");
    }

#if APP_MODE == MODE_CAMERA_DIAGNOSTICS
    runCameraTests();

#elif APP_MODE == MODE_STILL_IMAGE
    testSingleFrame();

#elif APP_MODE == MODE_IMAGE_STREAM_5S
    // Streaming is driven from loop().

#elif APP_MODE == MODE_VIDEO_STREAM
    // Streaming is driven from loop().

#else
#error "Unknown APP_MODE selected in AppConfig.hpp"
#endif
#endif
}

// ============================================================
// Loop
// ============================================================
void loop()
{
#if APP_MODE == MODE_IMAGE_STREAM_5S
    delay(5000);
    testSingleFrame();

#elif APP_MODE == MODE_VIDEO_STREAM
    testSingleFrame();

#else
    // One-shot modes: nothing to do here.
    delay(10000);
#endif
}
