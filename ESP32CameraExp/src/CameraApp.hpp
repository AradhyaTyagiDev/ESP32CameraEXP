#pragma once

#include <Arduino.h>
#include "esp_camera.h"

// ============================================================
// Camera application functions.
// Implementations live in camera_app.cpp (CameraApp.cpp).
// ============================================================

// Initialize the camera from the CameraConfig settings.
bool initCamera();

// Print sensor identification, image config, and memory info.
void printCameraStatus();

// Print board / MCU information (chip, CPU, flash, PSRAM).
void printMCUInfo();

// Capture and report a single frame (primed timing).
void captureTestFrame();

// Run an FPS benchmark for the given duration (ms).
void benchmarkFPS(uint32_t durationMs = 5000);

// ============================================================
// Camera tests
// Each test is an independent, self-contained check. Add new
// tests as functions here and register them in runCameraTests().
// ============================================================

// Report sensor identification, image config, and memory info.
void testCameraStatus();

// Capture and report a single frame (primed timing).
void testSingleFrame();

// Run the FPS benchmark for a fixed duration.
void testFps();

// Run all registered camera tests in sequence (sensor check + settle
// first). This is the single entry point called from setup().
void runCameraTests();

// Halt the system after printing a fatal error message.
void halt(const char *reason);
