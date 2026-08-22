#pragma once

// =====================================================================
//  APPLICATION MODE  -  change THIS ONE line (APP_MODE) to select what
//  the device does after boot. Board + camera are chosen separately
//  via the -DBOARD_PROFILE_* build flag in platformio.ini. Everything
//  else (which reports/handlers to run) is picked automatically.
//
//    MODE_BOARD_INFO          -> ESP32/board info only (no camera)
//    MODE_CAMERA_DIAGNOSTICS  -> full camera diagnostic suite
//    MODE_STILL_IMAGE         -> still image capture
//    MODE_IMAGE_STREAM_5S     -> image every 5s
//    MODE_VIDEO_STREAM        -> video stream (fast capture loop)
//    MODE_CAMERA_INSPECTOR    -> OV5640 inspector (sensor/PSRAM/res/
//                                AF probe). Totally separate from
//                                SccbScanner; requires the camera to
//                                be initialized first.
//
//  To add a new mode:
//    1. Add a `#define MODE_YOUR_MODE n` below.
//    2. Add an `#elif APP_MODE == MODE_YOUR_MODE` block in main.cpp.
// =====================================================================
#define MODE_CAMERA_DIAGNOSTICS 0
#define MODE_CAMERA_SCCB_SCANNER 1
#define MODE_CAMERA_INSPECTOR 2

#ifndef APP_MODE
#define APP_MODE MODE_CAMERA_SCCB_SCANNER
#endif

#ifndef BOOT_SERIAL_DELAY_MS
#define BOOT_SERIAL_DELAY_MS 1000
#endif
