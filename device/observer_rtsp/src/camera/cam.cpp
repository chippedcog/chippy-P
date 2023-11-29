#include <Arduino.h> // needed to include otherwise got 'Serial' not in scope errors
#include "esp_camera.h"
#include "camera/cam.h"
#include "camera/cam_pins.h"

CamManager::CamManager()
    : _cam_config{
          // --- pins
          .pin_pwdn = PWDN_GPIO_NUM,
          .pin_reset = RESET_GPIO_NUM,
          .pin_xclk = XCLK_GPIO_NUM,
          .pin_sscb_sda = SIOD_GPIO_NUM,
          .pin_sscb_scl = SIOC_GPIO_NUM,
          .pin_d7 = Y9_GPIO_NUM,
          .pin_d6 = Y8_GPIO_NUM,
          .pin_d5 = Y7_GPIO_NUM,
          .pin_d4 = Y6_GPIO_NUM,
          .pin_d3 = Y5_GPIO_NUM,
          .pin_d2 = Y4_GPIO_NUM,
          .pin_d1 = Y3_GPIO_NUM,
          .pin_d0 = Y2_GPIO_NUM,
          .pin_vsync = VSYNC_GPIO_NUM,
          .pin_href = HREF_GPIO_NUM,
          .pin_pclk = PCLK_GPIO_NUM,
          // --- timers/clocks
          .xclk_freq_hz = 20000000, // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
          .ledc_timer = LEDC_TIMER_0,
          .ledc_channel = LEDC_CHANNEL_0,
          // --- qualtiy settings
          .pixel_format = PIXFORMAT_JPEG, // for streaming
          .frame_size = FRAMESIZE_SXGA,   // sizes: https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
          .jpeg_quality = 45,             // 1 - 63 (lower numbers higher quality)
          // --- buffer refrences
          .fb_count = 1,                     // When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
          .fb_location = CAMERA_FB_IN_PSRAM, // trying to fix an init issue (https://github.com/espressif/esp32-camera/issues/571#issuecomment-1726196189)
          .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
      }
{
}

esp_err_t CamManager::initialize()
{
  Serial.println("[CamManager::initialize] esp_camera_init");
  // --- init
  esp_err_t camera_init_result = esp_camera_init(&_cam_config);
  if (camera_init_result != ESP_OK)
  {
    Serial.println("[CamManager::initialize] esp_camera_init FAILED");
    Serial.println(camera_init_result);
  }
  Serial.println("[CamManager::initialize] initial frame clearing");
  // --- clear first buffer: HACK: capture and throw away first frame. common issue with camera initialization/startup. the esp_camera_fb_return'ed a black frame or prior frame in loop below (did in setup, but after a crash doesn't seem to work): https://github.com/espressif/esp32-camera/issues/545#issuecomment-1600335819
  esp_camera_fb_return(esp_camera_fb_get());
  // --- return esp err/res
  return camera_init_result;
}

void CamManager::updateSettings()
{
  Serial.println("[CamManager::updateSettings] setting");
  sensor_t *s = esp_camera_sensor_get();
  s->set_brightness(s, 1);
  s->set_saturation(s, 1);
}

camera_fb_t *CamManager::capture()
{
  // --- capture
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
    Serial.println("[CamManager::capture] capture FAILED");
  else
    Serial.println("[CamManager::capture] capture");
  // --- return
  return fb;
}

void CamManager::release(camera_fb_t *fb)
{
  if (fb)
    esp_camera_fb_return(fb);
  else
    Serial.println("[CamManager::release] no frame buffer to release");
}