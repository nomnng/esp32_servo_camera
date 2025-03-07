#include "servo_controller.h"

#define SPEED_MODE LEDC_LOW_SPEED_MODE
#define PWM_PIN_1 13
#define PWM_PIN_2 14
#define TAG "servo_controller"

#define PWM_FREQUENCY 50

void init_servo_pwm() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = LEDC_TIMER_1,
        .freq_hz          = PWM_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = SPEED_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER_1,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PWM_PIN_1,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_channel.channel = LEDC_CHANNEL_2;
    ledc_channel.gpio_num = PWM_PIN_2;
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

// speed -> [-10, 10] (0 - stop, sign controls rotation direction)
void set_servo_speed(int speed, int channel) {
    if (speed < -10) {
        speed = -10;
    } else if (speed > 10) {
        speed = 10;
    }

    int ms_per_pulse = 1000 / PWM_FREQUENCY;
    int min_duty = 8192 / ms_per_pulse;
    int max_duty = (8192 * 2) / ms_per_pulse;
    int duty = min_duty + ((max_duty - min_duty) / 20) * (speed + 10);
    ESP_LOGI(TAG, "PWM DUTY: %d", duty);

    ESP_ERROR_CHECK(ledc_set_duty(SPEED_MODE, channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(SPEED_MODE, channel));
}

