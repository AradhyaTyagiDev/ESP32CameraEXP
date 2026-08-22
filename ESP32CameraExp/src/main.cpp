#include <Arduino.h>
#include "AppConfig.hpp"
#include "CameraApp.hpp"
#include "SccbScanner.hpp"
#include "CameraInspector.hpp"

// ============================================================
// Arduino setup
// ============================================================
void setup()
{
    Serial.begin(115200);
    delay(BOOT_SERIAL_DELAY_MS);

    printMCUInfo();

#if APP_MODE == MODE_CAMERA_DIAGNOSTICS
    runCameraTests();
#elif APP_MODE == MODE_CAMERA_SCCB_SCANNER
    // Pre-flight: scan SCCB bus and read the sensor ID (works even before esp_camera_init() claims the bus).
    SccbScanner().run();
#elif APP_MODE == MODE_CAMERA_INSPECTOR
    // --------------------------------------------------------
    // OV5640 Inspector (totally separate from SccbScanner).
    // Requires the camera to be initialized first.
    // --------------------------------------------------------
    if (!initCamera())
    {
        halt("CAMERA INIT FAILED.");
    }

    CameraInspector().run();

#else
#error "Unknown APP_MODE selected in AppConfig.hpp"
#endif
}

// ============================================================
// Loop
// ============================================================
void loop()
{
    // #if APP_MODE == MODE_IMAGE_STREAM_5S
    //     delay(5000);
    //     testSingleFrame();

    // #elif APP_MODE == MODE_VIDEO_STREAM
    //     testSingleFrame();

    // #else
    //     // One-shot modes: nothing to do here.
    //     delay(10000);
    // #endif

    delay(10000);
}
