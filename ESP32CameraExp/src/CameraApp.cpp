#include "CameraApp.hpp"
#include "CameraConfig.hpp"

// ============================================================
// Print camera status
// ============================================================
void printCameraStatus()
{
    sensor_t *s = esp_camera_sensor_get();

    if (s == nullptr)
    {
        Serial.println("ERROR: esp_camera_sensor_get() returned NULL");
        return;
    }

    Serial.println();
    Serial.println("==============================================");
    Serial.println("          OV5640 CAMERA STATUS");
    Serial.println("==============================================");

    // --------------------------------------------------------
    // Sensor identification
    // --------------------------------------------------------

    Serial.printf("Sensor PID       : 0x%04X\n", s->id.PID);
    Serial.printf("Sensor VER       : 0x%02X\n", s->id.VER);
    Serial.printf("Sensor MIDL      : 0x%02X\n", s->id.MIDL);
    Serial.printf("Sensor MIDH      : 0x%02X\n", s->id.MIDH);

    // --------------------------------------------------------
    // Image configuration
    // --------------------------------------------------------
    Serial.printf("Frame size enum  : %d\n", s->status.framesize);
    Serial.printf("JPEG quality     : %d\n", s->status.quality);

    Serial.printf("Brightness       : %d\n", s->status.brightness);
    Serial.printf("Contrast         : %d\n", s->status.contrast);
    Serial.printf("Saturation       : %d\n", s->status.saturation);
    Serial.printf("Sharpness        : %d\n", s->status.sharpness);

    // --------------------------------------------------------
    // Exposure / Gain / White Balance
    // --------------------------------------------------------

    Serial.printf("AEC              : %d\n", s->status.aec);
    Serial.printf("AGC              : %d\n", s->status.agc);
    Serial.printf("AWB              : %d\n", s->status.awb);

    Serial.printf("AEC2             : %d\n", s->status.aec2);
    Serial.printf("AGC gain         : %d\n", s->status.agc_gain);
    Serial.printf("AE level         : %d\n", s->status.ae_level);
    Serial.printf("WB mode          : %d\n", s->status.wb_mode);

    // --------------------------------------------------------
    // Orientation
    // --------------------------------------------------------

    Serial.printf("Mirror (H)       : %d\n", s->status.hmirror);
    Serial.printf("Flip (V)         : %d\n", s->status.vflip);

    // --------------------------------------------------------
    // PSRAM
    // --------------------------------------------------------

    Serial.println();
    Serial.println("----- MEMORY -----");

    Serial.printf("PSRAM found      : %s\n",
                  psramFound() ? "YES" : "NO");

    Serial.printf("PSRAM size       : %u bytes\n",
                  ESP.getPsramSize());

    Serial.printf("Free PSRAM       : %u bytes\n",
                  ESP.getFreePsram());

    Serial.printf("Free heap        : %u bytes\n",
                  ESP.getFreeHeap());

    Serial.println("==============================================");
    Serial.println();
}

// ============================================================
// Capture one frame
// ============================================================

void captureTestFrame()
{
    Serial.println();
    Serial.println("==============================================");
    Serial.println("             FRAME CAPTURE TEST");
    Serial.println("==============================================");

    // Prime the pipeline: with CAMERA_GRAB_LATEST a frame is already
    // queued in PSRAM, so the first get() returns instantly. Flush it so
    // the next get() blocks until a fresh frame arrives and the measured
    // time reflects the real frame period instead of retrieval overhead.
    camera_fb_t *prime = esp_camera_fb_get();
    if (prime)
        esp_camera_fb_return(prime);

    uint32_t start = millis();

    camera_fb_t *fb = esp_camera_fb_get();

    uint32_t elapsed = millis() - start;

    if (fb == nullptr)
    {
        Serial.println("ERROR: Frame capture FAILED");
        Serial.printf("Capture time: %lu ms\n", elapsed);
        return;
    }

    Serial.println("Frame capture SUCCESS");

    Serial.printf("Capture time    : %lu ms\n", elapsed);
    Serial.printf("Frame width     : %u\n", fb->width);
    Serial.printf("Frame height    : %u\n", fb->height);
    Serial.printf("Frame length    : %u bytes\n", fb->len);
    Serial.printf("Pixel format    : %d\n", fb->format);
    Serial.printf("Frame buffer    : %p\n", fb->buf);

    // A primed single capture approximates one frame period.
    // Guard against a near-zero elapsed (e.g. a still-queued frame),
    // which would otherwise yield an absurdly high FPS.
    if (elapsed >= 1)
    {
        Serial.printf("Approx FPS      : %.2f\n",
                      1000.0f / elapsed);
    }
    else
    {
        Serial.println("Approx FPS      : n/a (frame already queued)");
    }

    // Return buffer to camera driver
    esp_camera_fb_return(fb);

    Serial.println("Frame buffer returned.");
    Serial.println("==============================================");
}

// ============================================================
// FPS benchmark
// ============================================================
void benchmarkFPS(uint32_t durationMs)
{
    Serial.println();
    Serial.println("==============================================");
    Serial.println("               FPS BENCHMARK");
    Serial.println("==============================================");

    uint32_t start = millis();

    uint32_t frames = 0;
    uint32_t totalBytes = 0;

    uint32_t minFrameSize = UINT32_MAX;
    uint32_t maxFrameSize = 0;

    while (millis() - start < durationMs)
    {
        camera_fb_t *fb = esp_camera_fb_get();

        if (fb == nullptr)
        {
            Serial.println("Frame capture failed!");
            continue;
        }

        frames++;

        totalBytes += fb->len;

        if (fb->len < minFrameSize)
            minFrameSize = fb->len;

        if (fb->len > maxFrameSize)
            maxFrameSize = fb->len;

        esp_camera_fb_return(fb);
    }

    uint32_t elapsed = millis() - start;

    float fps = (frames * 1000.0f) / elapsed;

    float averageSize = 0;

    if (frames > 0)
        averageSize = totalBytes / (float)frames;

    Serial.printf("Duration        : %lu ms\n", elapsed);
    Serial.printf("Frames          : %lu\n", frames);
    Serial.printf("FPS             : %.2f\n", fps);

    if (frames > 0)
    {
        Serial.printf("Average JPEG    : %.2f KB\n",
                      averageSize / 1024.0f);

        Serial.printf("Minimum JPEG    : %.2f KB\n",
                      minFrameSize / 1024.0f);

        Serial.printf("Maximum JPEG    : %.2f KB\n",
                      maxFrameSize / 1024.0f);
    }

    Serial.println("==============================================");
}

// ============================================================
// Camera initialization
// ============================================================
bool initCamera()
{
    // All settings live in CameraConfig (CameraConfig.hpp).
    // Tweak fields there (or here) to change camera behavior.
    CameraConfig cam;

    camera_config_t config = cam.build();

    if (cam.usingPsram())
    {
        Serial.println("PSRAM detected.");
        Serial.println("Using PSRAM frame buffers.");
    }
    else
    {
        Serial.println("WARNING: PSRAM NOT detected.");
    }

    // --------------------------------------------------------
    // Initialize
    // --------------------------------------------------------

    Serial.println();
    Serial.println("Initializing camera...");

    esp_err_t err = esp_camera_init(&config);

    if (err != ESP_OK)
    {
        Serial.printf(
            "Camera initialization FAILED: 0x%X\n",
            err);

        return false;
    }

    Serial.println("Camera initialization SUCCESS.");

    return true;
}

void printMCUInfo()
{
    Serial.println();
    Serial.println();
    Serial.println("==============================================");
    Serial.println("       ESP32-S3 OV5640 CAMERA LAB");
    Serial.println("              CAM-01 INSPECTOR");
    Serial.println("==============================================");

    Serial.printf("Chip            : %s\n",
                  ESP.getChipModel());

    Serial.printf("CPU frequency   : %u MHz\n",
                  ESP.getCpuFreqMHz());

    Serial.printf("Flash size      : %u MB\n",
                  ESP.getFlashChipSize() / (1024 * 1024));

    Serial.printf("PSRAM size      : %u MB\n",
                  ESP.getPsramSize() / (1024 * 1024));
}

// ============================================================
// Halt the system after a fatal error
// ============================================================
void halt(const char *reason)
{
    Serial.println();
    Serial.println(reason);
    Serial.println("System halted.");

    while (true)
    {
        delay(1000);
    }
}

// ============================================================
// Camera tests
// ============================================================

void testCameraStatus()
{
    printCameraStatus();
}

void testSingleFrame()
{
    captureTestFrame();
}

void testFps()
{
    benchmarkFPS(5000);
}

void runCameraTests()
{
    sensor_t *s = esp_camera_sensor_get();

    if (s == nullptr)
    {
        Serial.println("ERROR: sensor_t is NULL.");
        return;
    }

    // Let auto exposure / white balance settle
    Serial.println();
    Serial.println("Allowing camera to settle...");

    for (int i = 0; i < 5; i++)
    {
        camera_fb_t *fb = esp_camera_fb_get();

        if (fb)
        {
            esp_camera_fb_return(fb);
        }

        delay(100);
    }

    // --- Tests ---
    testCameraStatus();
    testSingleFrame();
    testFps();

    Serial.println();
    Serial.println("==============================================");
    Serial.println("             CAM-01 COMPLETE");
    Serial.println("==============================================");
}
