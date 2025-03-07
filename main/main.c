#include <esp_log.h>
#include <esp_netif.h>
#include <esp_event.h>

#include <protocol_examples_common.h>
#include <esp_wifi.h>

#include "camera.h"
#include "nvs_utils.h"
#include "debug_tcp_server.h"
#include "command_handler.h"
#include "servo_controller.h"

#define TAG "main"

void app_main(void) {
    ESP_ERROR_CHECK(initialize_nvs());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_connect());
    esp_wifi_set_ps(WIFI_PS_NONE);

    camera_init();
    start_debug_tcp_server();
    start_command_handler();

    init_servo_pwm();

    ESP_LOGI(TAG, "Initialization competed successfully!");
}
