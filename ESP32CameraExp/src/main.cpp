#include "Arduino.h"
#include "esp_camera.h"

void setup() {
  Serial.begin(115200);

  sensor_t *s = esp_camera_sensor_get();

  Serial.printf("PID: 0x%04X\n", s->id.PID);
  Serial.printf("VER: 0x%02X\n", s->id.VER);

  Serial.printf("Frame size: %d\n", s->status.framesize);
  Serial.printf("JPEG quality: %d\n", s->status.quality);

  Serial.printf("Brightness: %d\n", s->status.brightness);
  Serial.printf("Contrast: %d\n", s->status.contrast);
  Serial.printf("Saturation: %d\n", s->status.saturation);
  Serial.printf("Sharpness: %d\n", s->status.sharpness);

  Serial.printf("AEC: %d\n", s->status.aec);
  Serial.printf("AGC: %d\n", s->status.agc);
  Serial.printf("AWB: %d\n", s->status.awb);

  Serial.printf("Mirror: %d\n", s->status.hmirror);
  Serial.printf("Flip: %d\n", s->status.vflip);
}

void loop() {
}
