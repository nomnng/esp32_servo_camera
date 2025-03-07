#include <driver/ledc.h>
#include <esp_log.h>

void init_servo_pwm();
void set_servo_speed(int speed, int channel);
