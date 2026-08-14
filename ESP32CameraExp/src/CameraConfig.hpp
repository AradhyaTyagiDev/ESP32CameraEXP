#pragma once

#include <Arduino.h>
#include "esp_camera.h"
#include "PinsConfig.hpp"

// ============================================================
// CameraConfig
// Single source of truth for all ESP32-S3 + OV5640 settings.
// Adjust any field here, then call build() to obtain the
// esp_camera driver configuration.
// ============================================================

class CameraConfig
{
public:
    // --------------------------------------------------------
    // XCLK generator (LEDC)
    // --------------------------------------------------------
    ledc_channel_t ledc_channel = LEDC_CHANNEL_0;
    ledc_timer_t   ledc_timer   = LEDC_TIMER_0;

    // --------------------------------------------------------
    // Pins (sourced from PinsConfig.hpp)
    // --------------------------------------------------------
    int pin_pwdn      = PWDN_GPIO_NUM;
    int pin_reset     = RESET_GPIO_NUM;
    int pin_xclk      = XCLK_GPIO_NUM;
    int pin_sccb_sda  = SIOD_GPIO_NUM;
    int pin_sccb_scl  = SIOC_GPIO_NUM;
    int pin_d0        = Y2_GPIO_NUM;
    int pin_d1        = Y3_GPIO_NUM;
    int pin_d2        = Y4_GPIO_NUM;
    int pin_d3        = Y5_GPIO_NUM;
    int pin_d4        = Y6_GPIO_NUM;
    int pin_d5        = Y7_GPIO_NUM;
    int pin_d6        = Y8_GPIO_NUM;
    int pin_d7        = Y9_GPIO_NUM;
    int pin_vsync     = VSYNC_GPIO_NUM;
    int pin_href      = HREF_GPIO_NUM;
    int pin_pclk      = PCLK_GPIO_NUM;

    // --------------------------------------------------------
    // SCCB (sensor control bus)
    // --------------------------------------------------------
    // -1 = drive SCCB directly on the pins (software bit-banged
    // bus) instead of claiming a hardware I2C port.
    int sccb_i2c_port = -1;

    // OV5640 7-bit SCCB/I2C address (see OV5640_SCCB_ADDR in PinsConfig.hpp).
    int sccb_addr = OV5640_SCCB_ADDR;

    // --------------------------------------------------------
    // Clock
    // --------------------------------------------------------
    int xclk_freq_hz = 20000000;

    // --------------------------------------------------------
    // Image
    // --------------------------------------------------------
    pixformat_t pixel_format = PIXFORMAT_JPEG;
    framesize_t frame_size   = FRAMESIZE_VGA;
    int jpeg_quality         = 12;

    // --------------------------------------------------------
    // Frame buffers (PSRAM present)
    // --------------------------------------------------------
    size_t                   fb_count        = 2;
    camera_fb_location_t     fb_location     = CAMERA_FB_IN_PSRAM;
    camera_grab_mode_t       grab_mode       = CAMERA_GRAB_LATEST;

    // --------------------------------------------------------
    // Frame buffers (no PSRAM fallback)
    // --------------------------------------------------------
    size_t                   fb_count_no_psram    = 1;
    camera_fb_location_t     fb_location_no_psram = CAMERA_FB_IN_DRAM;
    camera_grab_mode_t       grab_mode_no_psram   = CAMERA_GRAB_WHEN_EMPTY;

    // --------------------------------------------------------
    // Build the esp_camera driver configuration.
    // Selects the PSRAM or DRAM frame-buffer profile based on
    // hardware detection, so all settings stay in this class.
    // --------------------------------------------------------
    camera_config_t build() const
    {
        camera_config_t cfg = {};

        cfg.ledc_channel = ledc_channel;
        cfg.ledc_timer   = ledc_timer;

        cfg.pin_pwdn     = pin_pwdn;
        cfg.pin_reset    = pin_reset;
        cfg.pin_xclk     = pin_xclk;
        cfg.pin_sccb_sda = pin_sccb_sda;
        cfg.pin_sccb_scl = pin_sccb_scl;

        cfg.pin_d0 = pin_d0;
        cfg.pin_d1 = pin_d1;
        cfg.pin_d2 = pin_d2;
        cfg.pin_d3 = pin_d3;
        cfg.pin_d4 = pin_d4;
        cfg.pin_d5 = pin_d5;
        cfg.pin_d6 = pin_d6;
        cfg.pin_d7 = pin_d7;

        cfg.pin_vsync = pin_vsync;
        cfg.pin_href  = pin_href;
        cfg.pin_pclk  = pin_pclk;

        cfg.sccb_i2c_port = sccb_i2c_port;
        cfg.sccb_addr     = sccb_addr;
        cfg.xclk_freq_hz  = xclk_freq_hz;

        cfg.pixel_format = pixel_format;
        cfg.frame_size   = frame_size;
        cfg.jpeg_quality = jpeg_quality;

        if (psramFound())
        {
            cfg.fb_location = fb_location;
            cfg.fb_count    = fb_count;
            cfg.grab_mode   = grab_mode;
        }
        else
        {
            cfg.fb_location = fb_location_no_psram;
            cfg.fb_count    = fb_count_no_psram;
            cfg.grab_mode   = grab_mode_no_psram;
        }

        return cfg;
    }

    // True when the PSRAM frame-buffer profile will be used.
    bool usingPsram() const
    {
        return psramFound();
    }
};
