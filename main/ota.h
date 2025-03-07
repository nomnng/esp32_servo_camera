#include <esp_ota_ops.h>
#include <esp_log.h>

void abort_ota();
esp_err_t begin_ota(int new_firmware_size);
esp_err_t write_ota(char *buffer, int len);
esp_err_t end_ota();
