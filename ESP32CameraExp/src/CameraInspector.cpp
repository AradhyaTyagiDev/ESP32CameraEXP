#include "CameraInspector.hpp"

// ============================================================
// Static name tables (aligned with enum order in sensor.h)
// ============================================================

static const char *kPixFormatNames[] = {
    "RGB565",    // PIXFORMAT_RGB565
    "YUV422",    // PIXFORMAT_YUV422
    "YUV420",    // PIXFORMAT_YUV420
    "GRAYSCALE", // PIXFORMAT_GRAYSCALE
    "JPEG",      // PIXFORMAT_JPEG
    "RGB888",    // PIXFORMAT_RGB888
    "RAW",       // PIXFORMAT_RAW
    "RGB444",    // PIXFORMAT_RGB444
    "RGB555",    // PIXFORMAT_RGB555
};

static const char *kFrameSizeNames[] = {
    "96X96",   // FRAMESIZE_96X96
    "QQVGA",   // FRAMESIZE_QQVGA
    "QCIF",    // FRAMESIZE_QCIF
    "HQVGA",   // FRAMESIZE_HQVGA
    "240X240", // FRAMESIZE_240X240
    "QVGA",    // FRAMESIZE_QVGA
    "CIF",     // FRAMESIZE_CIF
    "HVGA",    // FRAMESIZE_HVGA
    "VGA",     // FRAMESIZE_VGA
    "SVGA",    // FRAMESIZE_SVGA
    "XGA",     // FRAMESIZE_XGA
    "HD",      // FRAMESIZE_HD
    "SXGA",    // FRAMESIZE_SXGA
    "UXGA",    // FRAMESIZE_UXGA
    "FHD",     // FRAMESIZE_FHD
    "P_HD",    // FRAMESIZE_P_HD
    "P_3MP",   // FRAMESIZE_P_3MP
    "QXGA",    // FRAMESIZE_QXGA
    "QHD",     // FRAMESIZE_QHD
    "WQXGA",   // FRAMESIZE_WQXGA
    "P_FHD",   // FRAMESIZE_P_FHD
    "QSXGA",   // FRAMESIZE_QSXGA
};

const char *CameraInspector::pixformatName(pixformat_t f)
{
    int i = (int)f;
    if (i >= 0 && i < (int)(sizeof(kPixFormatNames) / sizeof(kPixFormatNames[0])))
        return kPixFormatNames[i];
    return "UNKNOWN";
}

const char *CameraInspector::framesizeName(framesize_t fs)
{
    int i = (int)fs;
    if (i >= 0 && i < (int)(sizeof(kFrameSizeNames) / sizeof(kFrameSizeNames[0])))
        return kFrameSizeNames[i];
    return "INVALID";
}

// ============================================================
// Sensor identity
// ============================================================
void CameraInspector::reportSensorIdentity(sensor_t *s)
{
    Serial.println();
    Serial.println("----- SENSOR IDENTITY -----");

    Serial.printf("Sensor PID       : 0x%04X\n", s->id.PID);
    Serial.printf("Sensor VER       : 0x%02X\n", s->id.VER);
    Serial.printf("Sensor MID       : 0x%02X%02X\n", s->id.MIDH, s->id.MIDL);
    Serial.printf("SCCB address     : 0x%02X (expected 0x%02X)\n",
                  s->slv_addr, OV5640_SCCB_ADDR);

    if (s->id.PID == OV5640_PID)
    {
        Serial.println("Sensor model     : OV5640 DETECTED");
    }
    else
    {
        Serial.printf("Sensor model     : NOT OV5640 (PID 0x%04X)\n", s->id.PID);
        Serial.println("WARNING: this inspector assumes an OV5640.");
    }
}

// ============================================================
// PSRAM
// ============================================================
void CameraInspector::reportPsram()
{
    Serial.println();
    Serial.println("----- PSRAM -----");

    Serial.printf("PSRAM found      : %s\n",
                  psramFound() ? "YES" : "NO");
    Serial.printf("PSRAM size       : %u bytes (%u MB)\n",
                  ESP.getPsramSize(),
                  ESP.getPsramSize() / (1024 * 1024));
    Serial.printf("Free PSRAM       : %u bytes\n", ESP.getFreePsram());
    Serial.printf("Free heap        : %u bytes\n", ESP.getFreeHeap());
}

// ============================================================
// Supported resolutions
// ============================================================
void CameraInspector::reportSupportedResolutions(sensor_t *s)
{
    Serial.println();
    Serial.println("----- SUPPORTED RESOLUTIONS -----");

    // Find the sensor's maximum resolution from the driver's table.
    framesize_t maxFs = FRAMESIZE_QSXGA; // OV5640 upper bound as a safe default
    camera_sensor_info_t *info = esp_camera_sensor_get_info(&s->id);
    if (info != nullptr)
    {
        maxFs = info->max_size;
    }

    Serial.printf("Max framesize    : %s\n\n",
                  framesizeName(maxFs));

    // Iterate the static resolution[] table (indexed by framesize_t).
    for (int i = 0; i <= (int)maxFs && i < (int)FRAMESIZE_INVALID; i++)
    {
        framesize_t fs = (framesize_t)i;
        Serial.printf("  %-10s : %4u x %4u\n",
                      framesizeName(fs),
                      resolution[fs].width,
                      resolution[fs].height);
    }
}

// ============================================================
// Pixel format
// ============================================================
void CameraInspector::reportPixelFormat(sensor_t *s)
{
    Serial.println();
    Serial.println("----- PIXEL FORMAT -----");

    Serial.printf("Current format   : %s\n", pixformatName(s->pixformat));
    Serial.printf("Framesize enum   : %s (%d)\n",
                  framesizeName(s->status.framesize),
                  s->status.framesize);
}

// ============================================================
// Image settings (JPEG quality etc.)
// ============================================================
void CameraInspector::reportImageSettings(sensor_t *s)
{
    Serial.println();
    Serial.println("----- IMAGE SETTINGS -----");

    Serial.printf("JPEG quality     : %d (0=best .. 63=worst)\n",
                  s->status.quality);
    Serial.printf("Brightness       : %d\n", s->status.brightness);
    Serial.printf("Contrast         : %d\n", s->status.contrast);
    Serial.printf("Saturation       : %d\n", s->status.saturation);
    Serial.printf("Sharpness        : %d\n", s->status.sharpness);
    Serial.printf("Denoise          : %d\n", s->status.denoise);
    Serial.printf("Special effect   : %d\n", s->status.special_effect);
}

// ============================================================
// Exposure / gain / white balance
// ============================================================
void CameraInspector::reportExposureGainWB(sensor_t *s)
{
    Serial.println();
    Serial.println("----- EXPOSURE / GAIN / WB -----");

    Serial.printf("AEC (auto exp)   : %s\n", s->status.aec ? "ON" : "OFF");
    Serial.printf("AEC2             : %s\n", s->status.aec2 ? "ON" : "OFF");
    Serial.printf("AE level         : %d\n", s->status.ae_level);
    Serial.printf("AEC value (man)  : %d\n", s->status.aec_value);

    Serial.printf("AGC (auto gain)  : %s\n", s->status.agc ? "ON" : "OFF");
    Serial.printf("AGC gain         : %d\n", s->status.agc_gain);

    Serial.printf("AWB (auto WB)    : %s\n", s->status.awb ? "ON" : "OFF");
    Serial.printf("AWB gain         : %s\n", s->status.awb_gain ? "ON" : "OFF");
    Serial.printf("WB mode          : %d\n", s->status.wb_mode);
}

// ============================================================
// Mirror / flip
// ============================================================
void CameraInspector::reportMirrorFlip(sensor_t *s)
{
    Serial.println();
    Serial.println("----- ORIENTATION -----");

    Serial.printf("Mirror (H)       : %s\n", s->status.hmirror ? "ON" : "OFF");
    Serial.printf("Flip (V)         : %s\n", s->status.vflip ? "ON" : "OFF");
}

// ============================================================
// Capture one frame
// ============================================================
void CameraInspector::captureOneFrame()
{
    Serial.println();
    Serial.println("----- SINGLE FRAME CAPTURE -----");

    // Prime the pipeline so the measured time reflects a fresh frame.
    camera_fb_t *prime = esp_camera_fb_get();
    if (prime)
        esp_camera_fb_return(prime); // Return the frame buffer to be reused again.

    uint32_t start = millis();
    camera_fb_t *fb = esp_camera_fb_get();
    uint32_t elapsed = millis() - start;

    if (fb == nullptr)
    {
        Serial.println("ERROR: frame capture FAILED");
        return;
    }

    Serial.println("Frame capture SUCCESS");
    Serial.printf("Resolution      : %u x %u\n", fb->width, fb->height);
    Serial.printf("Pixel format    : %s\n", pixformatName(fb->format));
    Serial.printf("JPEG size       : %u bytes (%.2f KB)\n",
                  fb->len, fb->len / 1024.0f);
    Serial.printf("Capture time     : %lu ms\n", elapsed);

    esp_camera_fb_return(fb);
}

// ============================================================
// Measure actual capture FPS
// ============================================================
// This function benchmarks the camera by repeatedly grabbing frames
// for a fixed time window and measuring how many frames are produced.
// It tells us the real-world FPS of the sensor + driver + memory path,
// not just the theoretical camera clock speed.
//
// Practical idea:
//   - grab one frame
//   - count it
//   - note its size
//   - return it to the driver buffer pool
//   - repeat until the measurement time is over
//   - then compute FPS and average frame size
void CameraInspector::measureFps(uint32_t durationMs)
{
    Serial.println();
    Serial.println("----- CAPTURE FPS MEASUREMENT -----");

    // Start time for the test window.
    uint32_t start = millis();

    // Counters for results.
    uint32_t frames = 0;          // total number of successful frames captured
    uint32_t totalBytes = 0;      // sum of frame sizes in bytes
    uint32_t minLen = UINT32_MAX; // smallest frame seen
    uint32_t maxLen = 0;          // largest frame seen

    // Keep grabbing frames for the selected duration.
    while (millis() - start < durationMs)
    {
        // Request the next available frame buffer from the camera driver.
        camera_fb_t *fb = esp_camera_fb_get();

        // If the camera did not provide a frame, skip it.
        if (fb == nullptr)
        {
            Serial.println("Frame capture FAILED during benchmark.");
            continue;
        }

        // A valid frame was captured, so count it.
        frames++;
        totalBytes += fb->len;

        // Track min/max frame size for a rough size distribution.
        if (fb->len < minLen)
            minLen = fb->len;
        if (fb->len > maxLen)
            maxLen = fb->len;

        // IMPORTANT:
        // Return the buffer immediately so the driver can reuse it.
        // Otherwise the camera buffer pool will fill and capture will stall.
        esp_camera_fb_return(fb);
    }

    // Time elapsed for the benchmark.
    uint32_t elapsed = millis() - start;

    // Compute actual FPS.
    // Example:
    //   50 frames in 5000 ms => 10 FPS
    float fps = (frames * 1000.0f) / elapsed;

    // Average frame size.
    float avg = frames > 0 ? totalBytes / (float)frames : 0;

    // Print results.
    Serial.printf("Duration         : %lu ms\n", elapsed);
    Serial.printf("Frames           : %lu\n", frames);
    Serial.printf("Actual FPS       : %.2f\n", fps);

    if (frames > 0)
    {
        Serial.printf("Average size     : %.2f KB\n", avg / 1024.0f);
        Serial.printf("Min / Max size   : %.2f / %.2f KB\n",
                      minLen / 1024.0f, maxLen / 1024.0f);
    }
}

// ============================================================
// Autofocus verification (best-effort heuristic)
//
// The OV5640 die is autofocus-capable only when mounted on a lens
// module with a voice-coil motor (VCM). This driver/framework
// version does NOT expose set_focus/set_autofocus, so we probe the
// VCM focus-position register (0x3024) directly via set_reg/get_reg:
// a value we write and read back indicates a writable VCM => AF lens.
// This is a heuristic, not a 100% guarantee of a moving lens.
// ============================================================
void CameraInspector::verifyAutofocus(sensor_t *s)
{
    Serial.println();
    Serial.println("----- AUTOFOCUS VERIFICATION -----");

    // 1) Sensor family check.
    bool ov5640 = (s->id.PID == OV5640_PID);
    Serial.printf("OV5640 family    : %s\n", ov5640 ? "YES" : "NO");
    if (!ov5640)
    {
        Serial.println("This sensor is not an OV5640; no VCM AF expected.");
        return;
    }

    // 2) Driver API limitation (this framework version exposes no AF hooks).
    //    The sensor_t in this esp_camera build has no set_focus/set_autofocus
    //    pointers, so any AF must be driven via raw SCCB registers below.
    Serial.println("Driver exposes NO AF control API in this framework version.");
    Serial.println("AF (if present) must be driven via raw SCCB registers.");

    // 3) Live VCM register probe (heuristic).
    if (s->get_reg == nullptr || s->set_reg == nullptr)
    {
        Serial.println("set_reg/get_reg unavailable; cannot probe VCM.");
        return;
    }

    const int kFocusReg = 0x3024; // OV5640 VCM focus position (low byte)
    const int kTestVal = 0x55;

    int orig = s->get_reg(s, kFocusReg, 0xFF);
    s->set_reg(s, kFocusReg, 0xFF, kTestVal);
    int after = s->get_reg(s, kFocusReg, 0xFF);
    s->set_reg(s, kFocusReg, 0xFF, (uint8_t)orig); // restore

    Serial.printf("VCM reg 0x%04X   : orig=0x%02X after=0x%02X\n",
                  kFocusReg, orig & 0xFF, after & 0xFF);

    if (after == kTestVal)
    {
        Serial.println("RESULT           : writable VCM register => "
                       "autofocus lens module PRESENT (high confidence).");
    }
    else
    {
        Serial.println("RESULT           : VCM register not writable => "
                       "fixed-focus module, or AF not wired (no VCM).");
    }
}

// ============================================================
// Run the full suite
// ============================================================
void CameraInspector::run()
{
    Serial.println();
    Serial.println("==============================================");
    Serial.println("           OV5640 CAMERA INSPECTOR");
    Serial.println("==============================================");

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr)
    {
        Serial.println("ERROR: esp_camera_sensor_get() returned NULL.");
        Serial.println("Make sure the camera is initialized before inspecting.");
        return;
    }

    // Let auto-exposure / white-balance settle for stable readings.
    Serial.println("Allowing camera to settle...");
    for (uint32_t i = 0; i < SETTLE_FRAMES; i++)
    {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb)
            esp_camera_fb_return(fb);
        delay(SETTLE_DELAY_MS);
    }

    reportSensorIdentity(s);
    reportPsram();
    reportSupportedResolutions(s);
    reportPixelFormat(s);
    reportImageSettings(s);
    reportExposureGainWB(s);
    reportMirrorFlip(s);

    captureOneFrame();
    measureFps(5000);
    verifyAutofocus(s);

    Serial.println();
    Serial.println("==============================================");
    Serial.println("             INSPECTION COMPLETE");
    Serial.println("==============================================");
}
