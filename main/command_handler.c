#include "command_handler.h"

#define PORT 7788
#define IP "192.168.0.103"
#define TAG "command_handler"

static int default_servo_speed = 8;

static void camera_capture_task(void *pvParameters) {
	int sock = (int) pvParameters;
	while (1) {
	    camera_fb_t* fb = camera_capture();
	    if (fb) {
	        send(sock, (void*) &fb->len, sizeof(fb->len), 0);

	        int send_size = 1024 * 16;
	        for (int sended = 0; sended < fb->len;) {
	            int to_send = (sended + send_size) > fb->len ? (fb->len - sended) : send_size;
	            int send_result = send(sock, fb->buf + sended, to_send, 0);
	            if (send_result <= 0) {
	            	break;
	            }
	            sended += send_result;
	        }

	        camera_fb_return(fb);
	    }

	    vTaskDelay(150 / portTICK_PERIOD_MS);
	}
}

static void handle_commands_from_server(int sock, struct sockaddr *dest_addr) {
    int err = connect(sock, dest_addr, sizeof(struct sockaddr));
    if (err != 0) {
    	if (errno != 104) {
	        ESP_LOGE(TAG, "Unable to connect: errno %d", errno);
    	}
	    vTaskDelay(1000 / portTICK_PERIOD_MS);
        return;
    }

    TaskHandle_t camera_capture_task_handle;
    xTaskCreate(camera_capture_task, "camera_capture", 4096, (void*) sock, 5, &camera_capture_task_handle);

    int len;
    char packet_type;

    do {
        len = recv(sock, &packet_type, sizeof(packet_type), 0);
        if (len <= 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            break;
        }

        if (packet_type >= 16 && packet_type < (16 + 16)) {
            int frame_size = packet_type - 16;
            ESP_LOGI(TAG, "Changed camera frame size to %d", frame_size);
            camera_change_framesize(frame_size);
        } else if (packet_type == 50) {
        	int speed = 0;
            ESP_LOGI(TAG, "Receiving servo speed");
			recv(sock, &speed, sizeof(speed), 0);
			set_servo_speed(speed, 1);
            ESP_LOGI(TAG, "Changed servo speed to %d", speed);
        } else if (packet_type == 51) {
        	int speed = 0;
            ESP_LOGI(TAG, "Receiving servo speed");
			recv(sock, &speed, sizeof(speed), 0);
			set_servo_speed(speed, 2);
            ESP_LOGI(TAG, "Changed servo speed to %d", speed);
        } else if (packet_type == 60) {
			set_servo_speed(-default_servo_speed, 1);
			set_servo_speed(default_servo_speed, 2);
            ESP_LOGI(TAG, "Moving forward");
        } else if (packet_type == 61) {
			set_servo_speed(default_servo_speed, 1);
			set_servo_speed(default_servo_speed, 2);
            ESP_LOGI(TAG, "Right rotation");
        } else if (packet_type == 62) {
			set_servo_speed(default_servo_speed, 1);
			set_servo_speed(-default_servo_speed, 2);
            ESP_LOGI(TAG, "Moving back");
        } else if (packet_type == 63) {
			set_servo_speed(-default_servo_speed, 1);
			set_servo_speed(-default_servo_speed, 2);
            ESP_LOGI(TAG, "Left rotation");
        } else if (packet_type == 64) {
			set_servo_speed(0, 1);
			set_servo_speed(0, 2);
            ESP_LOGI(TAG, "Stop");
        }
    } while (1);

    if (camera_capture_task_handle != NULL) {
	    vTaskDelete(camera_capture_task_handle);
    }
}

static void command_receiver_task(void *pvParameters) {
	struct sockaddr_in dest_addr;
	inet_pton(AF_INET, IP, &dest_addr.sin_addr);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);

	while (1) {
		int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	    if (sock < 0) {
	        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
	        vTaskDelete(NULL);
	        return;
	    }

	    handle_commands_from_server(sock, (struct sockaddr *)&dest_addr);

        shutdown(sock, 0);
        close(sock);
	}
}

void start_command_handler() {
    xTaskCreate(command_receiver_task, "command_handler", 4096, NULL, 5, NULL);
}
