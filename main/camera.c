#include "camera.h"

#define TAG "camera"

static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 10000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_UXGA,

    .jpeg_quality = 12,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};

esp_err_t camera_init() {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera initialization failed: errno %d", err);
        return err;
    }

    int frame_size;
    read_i32_from_nvs("main", "cam_img_size", &frame_size, FRAMESIZE_CIF);

    camera_change_framesize(frame_size);

    return ESP_OK;
}

esp_err_t camera_change_framesize(int frame_size) {
    sensor_t* sensor = esp_camera_sensor_get();
    sensor->set_framesize(sensor, frame_size);
    write_i32_to_nvs("main", "cam_img_size", frame_size);
    return ESP_OK;
}

camera_fb_t* camera_capture() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera Capture Failed");
    }

    return fb;
}

void camera_fb_return(camera_fb_t *fb) {
    esp_camera_fb_return(fb);
}
