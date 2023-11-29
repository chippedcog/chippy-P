// camera.h
#ifndef __cam_h__
#define __cam_h__

#include "esp_camera.h"

class CamManager
{
public:
  // --- constructor
  CamManager();
  // --- setup/config
  esp_err_t initialize();
  void updateSettings();
  // --- actions
  camera_fb_t *capture();
  void release(camera_fb_t *fb);

private:
  camera_config_t _cam_config;
  camera_model_t _cam_model = CAMERA_OV5640;
};

#endif // __cam_h__