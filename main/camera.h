#include <esp_log.h>
#include <esp_camera.h>

#include "nvs_utils.h"

// pinout for ESP32 WROVER board
#define CAM_PIN_PWDN    -1 // power down is not used
#define CAM_PIN_RESET   -1 // software reset will be performed
#define CAM_PIN_XCLK    21
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      19
#define CAM_PIN_D2      18
#define CAM_PIN_D1       5
#define CAM_PIN_D0       4
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

esp_err_t camera_init();
esp_err_t camera_change_framesize(int frame_size);
camera_fb_t* camera_capture();
void camera_fb_return(camera_fb_t *fb);
