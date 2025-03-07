#include <nvs_flash.h>
#include <esp_log.h>
#include <nvs.h>

esp_err_t initialize_nvs();
esp_err_t read_i32_from_nvs(const char* namespace, const char* key, int32_t* out_value, int32_t default_value);
esp_err_t write_i32_to_nvs(const char* namespace, const char* key, int32_t value);