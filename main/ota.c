#include "ota.h"

#define TAG "OTA"

static esp_ota_handle_t ota_update_handle = 0;
static esp_partition_t *ota_partition = NULL;

void abort_ota() {
    esp_ota_abort(ota_update_handle);
}

esp_err_t begin_ota(int new_firmware_size) {
    ota_partition = esp_ota_get_next_update_partition(NULL);
    if (!ota_partition) {
        ESP_LOGE(TAG, "OTA partition not found");
        return ESP_ERR_NOT_FOUND;
    }

    if (ota_partition->size < new_firmware_size) {
        ESP_LOGE(TAG, "New firmware is too big for ota partition, aborting ota update");
        return ESP_ERR_INVALID_SIZE;
    }

    esp_err_t err = esp_ota_begin(ota_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while beginning ota update: errno %d", err);
        abort_ota();
    }

    return err;
}

esp_err_t write_ota(char *buffer, int len) {
    esp_err_t err = esp_ota_write(ota_update_handle, buffer, len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while writing ota firmware: errno %d", err);
        abort_ota();
    }    
    
    return err;
}

esp_err_t end_ota() {
    esp_err_t err = esp_ota_end(ota_update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while ending ota update: errno %d", err);
        abort_ota();
        return err;
    }

    err = esp_ota_set_boot_partition(ota_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error occurred while changing boot partition: errno %d", err);
        abort_ota();
    }

    return err;
}

