#include <Arduino.h>
#include "esp_camera.h"

// ============================================================
// ESP32-S3 N16R8 + OV5640
// Camera pin configuration
// ============================================================

#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1

#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13

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

    if (fb->len > 0)
    {
        Serial.printf("Approx FPS      : %.2f\n",
                      1000.0f / elapsed);
    }

    // Return buffer to camera driver
    esp_camera_fb_return(fb);

    Serial.println("Frame buffer returned.");
    Serial.println("==============================================");
}

// ============================================================
// FPS benchmark
// ============================================================
void benchmarkFPS(uint32_t durationMs = 5000)
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
    camera_config_t config = {};

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;

    // Camera data pins
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;

    // Camera clock / sync
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;

    // SCCB
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    // Power / reset
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    // SCCB I2C port: -1 means drive SCCB directly on the configured pins
    // (software bit-banged bus) rather than claiming hardware I2C port 0.
    config.sccb_i2c_port = -1;

    // XCLK
    config.xclk_freq_hz = 20000000;

    // --------------------------------------------------------
    // Initial image configuration
    // --------------------------------------------------------

    config.pixel_format = PIXFORMAT_JPEG;

    // Start conservatively.
    // We will change this later.
    config.frame_size = FRAMESIZE_VGA;

    config.jpeg_quality = 12;

    // --------------------------------------------------------
    // PSRAM configuration
    // --------------------------------------------------------

    if (psramFound())
    {
        config.fb_location = CAMERA_FB_IN_PSRAM;

        config.fb_count = 2;

        config.grab_mode = CAMERA_GRAB_LATEST;

        Serial.println("PSRAM detected.");
        Serial.println("Using PSRAM frame buffers.");
    }
    else
    {
        config.fb_location = CAMERA_FB_IN_DRAM;

        config.fb_count = 1;

        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

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

// ============================================================
// Arduino setup
// ============================================================
void setup()
{
    Serial.begin(115200);

    delay(2000);

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

    // --------------------------------------------------------
    // Initialize camera
    // --------------------------------------------------------

    if (!initCamera())
    {
        Serial.println();
        Serial.println("CAMERA INIT FAILED.");
        Serial.println("System halted.");

        while (true)
        {
            delay(1000);
        }
    }

    // --------------------------------------------------------
    // Get sensor
    // --------------------------------------------------------

    sensor_t *s = esp_camera_sensor_get();

    if (s == nullptr)
    {
        Serial.println("ERROR: sensor_t is NULL.");
        return;
    }

    // --------------------------------------------------------
    // Let auto exposure / white balance settle
    // --------------------------------------------------------

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

    // --------------------------------------------------------
    // Print camera information
    // --------------------------------------------------------

    printCameraStatus();

    // --------------------------------------------------------
    // Capture test frame
    // --------------------------------------------------------

    captureTestFrame();

    // --------------------------------------------------------
    // FPS test
    // --------------------------------------------------------

    benchmarkFPS(5000);

    Serial.println();
    Serial.println("==============================================");
    Serial.println("             CAM-01 COMPLETE");
    Serial.println("==============================================");
}

// ============================================================
// Loop
// ============================================================

void loop()
{
    delay(5000);

    Serial.println();
    Serial.println("Running another capture test...");

    captureTestFrame();
}