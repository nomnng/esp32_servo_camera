#include "debug_tcp_server.h"

#define PORT 12345
#define TAG "debug_tcp_server"

static void download_and_apply_ota(const int socket) {
    unsigned int new_firmware_size = 0;
    int len = recv(socket, &new_firmware_size, sizeof(new_firmware_size), 0);
    if (len != sizeof(new_firmware_size)) {
        ESP_LOGE(TAG, "Error occurred while receiving ota firmware size: errno %d", len);
        return;
    }

    if (begin_ota(new_firmware_size) != ESP_OK) {
        return;
    }

    char buffer[512];

    for (int written = 0; written < new_firmware_size;) {
        int to_receive = (written + sizeof(buffer)) > new_firmware_size ? (new_firmware_size - written) : sizeof(buffer);
        len = recv(socket, buffer, to_receive, 0);
        if (len <= 0) {
            ESP_LOGE(TAG, "Error occurred while receiving ota firmware: errno %d", len);
            abort_ota();
            return;
        }

        written += len;
        if (write_ota(buffer, len) != ESP_OK) {
            return;
        }
    };

    if (end_ota() != ESP_OK) {
        return;
    }

    ESP_LOGI(TAG, "OTA update completed successfully, restarting the device...");
    shutdown(socket, 0);
    close(socket);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    esp_restart();
}

static void send_image_from_camera(int socket) {
    camera_fb_t* fb = camera_capture();
    if (fb) {
        send(socket, (void*) &fb->len, sizeof(fb->len), 0);

        int send_size = 1024 * 16;
        for (int sended = 0; sended < fb->len;) {
            int to_send = (sended + send_size) > fb->len ? (fb->len - sended) : send_size;
            sended += send(socket, fb->buf + sended, to_send, 0);
        }
        camera_fb_return(fb);
    }

}

static void receive_loop(const int socket) {
    int len;
    char packet_type;

    do {
        len = recv(socket, &packet_type, sizeof(packet_type), 0);
        if (len <= 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            break;
        }

        if (packet_type == 100) {
            ESP_LOGI(TAG, "Sending camera photo!");
            send_image_from_camera(socket);
        } else if (packet_type == 101) {
            ESP_LOGI(TAG, "Starting ota update!");
            download_and_apply_ota(socket);
        } else if (packet_type >= 16 && packet_type < (16 + 16)) {
            int frame_size = packet_type - 16;
            ESP_LOGI(TAG, "Changed camera frame size to %d", frame_size);
            camera_change_framesize(frame_size);
        }
    } while (1);
}

static void tcp_server_task(void *pvParameters) {
    struct sockaddr_storage dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", AF_INET);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr;
        socklen_t addr_len = sizeof(source_addr);
        int socket = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (socket < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        receive_loop(socket);

        shutdown(socket, 0);
        close(socket);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void start_debug_tcp_server() {
    xTaskCreate(tcp_server_task, "debug_server", 4096, NULL, 5, NULL);
}
