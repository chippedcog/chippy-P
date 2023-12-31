// Device lib
#include "esp_camera.h"
// Definitions file with consts below
#include "camera.h"
#include "pins_camera.h"

// a global variable dictated by the header file using extern
camera_config_t camera_config;

void camera_config_init()
{
  camera_config.ledc_channel = LEDC_CHANNEL_0;
  camera_config.ledc_timer = LEDC_TIMER_0;
  camera_config.pin_d0 = Y2_GPIO_NUM;
  camera_config.pin_d1 = Y3_GPIO_NUM;
  camera_config.pin_d2 = Y4_GPIO_NUM;
  camera_config.pin_d3 = Y5_GPIO_NUM;
  camera_config.pin_d4 = Y6_GPIO_NUM;
  camera_config.pin_d5 = Y7_GPIO_NUM;
  camera_config.pin_d6 = Y8_GPIO_NUM;
  camera_config.pin_d7 = Y9_GPIO_NUM;
  camera_config.pin_xclk = XCLK_GPIO_NUM;
  camera_config.pin_pclk = PCLK_GPIO_NUM;
  camera_config.pin_vsync = VSYNC_GPIO_NUM;
  camera_config.pin_href = HREF_GPIO_NUM;
  camera_config.pin_sscb_sda = SIOD_GPIO_NUM;
  camera_config.pin_sscb_scl = SIOC_GPIO_NUM;
  camera_config.pin_pwdn = PWDN_GPIO_NUM;
  camera_config.pin_reset = RESET_GPIO_NUM;
  camera_config.xclk_freq_hz = 20000000;
  camera_config.frame_size = FRAMESIZE_SXGA;   // 1280 x 1024, other sizes check https://randomnerdtutorials.com/esp32-cam-ov2640-camera-settings/
  camera_config.pixel_format = PIXFORMAT_JPEG; // for streaming
  // camera_config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  camera_config.fb_location = CAMERA_FB_IN_PSRAM;
  camera_config.jpeg_quality = 32; // 1 - 63
  camera_config.fb_count = 1;
  camera_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  // why not CAMERA_GRAB_LATEST??? https://github.com/espressif/esp32-camera/blob/master/driver/include/esp_camera.h#L88
  // I tried this change, but then I just kept getting a black frame or poor quality frame https://github.com/espressif/esp32-camera/issues/545#issuecomment-1606970861
}
