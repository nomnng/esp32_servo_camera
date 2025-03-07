#include "nvs_utils.h"

#define TAG "nvs_utils"

esp_err_t initialize_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
	return err;
}

esp_err_t read_i32_from_nvs(const char* namespace, const char* key, int32_t* out_value, int32_t default_value) {
	*out_value = default_value;

	nvs_handle_t handle;
	esp_err_t err = nvs_open(namespace, NVS_READONLY, &handle);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while opening nvs handle: errno %d", err);
		return err;
	}

	err = nvs_get_i32(handle, key, out_value);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while reading from nvs: errno %d", err);		
	}

	nvs_close(handle);
	return err;
}

esp_err_t write_i32_to_nvs(const char* namespace, const char* key, int32_t value) {
	nvs_handle_t handle;
	esp_err_t err = nvs_open(namespace, NVS_READWRITE, &handle);
	if (err != ESP_OK) {
		return err;
	}

	err = nvs_set_i32(handle, key, value);
	if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while writing to nvs: errno %d", err);
	} else {
		err = nvs_commit(handle);
		if (err != ESP_OK) {
	        ESP_LOGE(TAG, "Error occurred while commiting nvs changes: errno %d", err);
		}
	}

	nvs_close(handle);
	return err;
}
