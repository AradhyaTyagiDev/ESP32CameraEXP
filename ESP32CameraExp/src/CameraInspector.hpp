#pragma once

#include <Arduino.h>
#include "esp_camera.h"
#include "PinsConfig.hpp"

// ============================================================
// CameraInspector
// Standalone OV5640 inspection module. Unlike SccbScanner (which
// runs BEFORE esp_camera_init() on the raw SCCB bus), this class
// operates on a fully-initialized camera via the esp_camera API.
//
// It reports:
//   - OV5640 detection + sensor PID/version
//   - PSRAM detection and size
//   - all resolutions supported by the sensor
//   - current pixel format and JPEG quality
//   - exposure / gain / white-balance status
//   - mirror / flip orientation
//   - a single captured frame (resolution + JPEG size)
//   - measured real-world capture FPS
//   - best-effort autofocus capability verification
//
// Requires the camera to be initialized (call initCamera() first).
// ============================================================
class CameraInspector
{
public:
    // Run the full inspection suite. Must be called after
    // esp_camera_init() has succeeded.
    void run();

private:
    // ---- individual report steps ----
    void reportSensorIdentity(sensor_t *s);
    void reportPsram();
    void reportSupportedResolutions(sensor_t *s);
    void reportPixelFormat(sensor_t *s);
    void reportImageSettings(sensor_t *s);
    void reportExposureGainWB(sensor_t *s);
    void reportMirrorFlip(sensor_t *s);
    void captureOneFrame();
    void measureFps(uint32_t durationMs);
    void verifyAutofocus(sensor_t *s);

    // ---- helpers ----
    static const char *pixformatName(pixformat_t f);
    static const char *framesizeName(framesize_t fs);

    // Allow the settle loop before capturing.
    static const uint32_t SETTLE_FRAMES = 5;
    static const uint32_t SETTLE_DELAY_MS = 100;
};
