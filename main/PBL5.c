#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define LED_PIN         4
#define PUSH_BUTTON_PIN 5
static const char *TAG = "BUTTON_PWM_FADE";

#define LEDC_TIMER       LEDC_TIMER_0
#define LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL     LEDC_CHANNEL_0
#define LEDC_DUTY_RES    LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY   5000

static bool led_on = false;

// Khởi tạo PWM
void pwm_init() {
    ledc_timer_config_t timer_conf = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf = {
        .gpio_num       = LED_PIN,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&channel_conf);
}

// Set độ sáng LED
void set_led_brightness(uint8_t duty) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

// Task kiểm tra button
void button_task(void *pvParameter) {
    gpio_set_direction(PUSH_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PUSH_BUTTON_PIN, GPIO_PULLDOWN_ONLY);

    int last_button = 0;
    while(1) {
        int btn = gpio_get_level(PUSH_BUTTON_PIN);
        if (btn && !last_button) {
            led_on = true;  // Bật LED khi nhấn
            ESP_LOGI(TAG, "Button PRESSED - LED ON");
        }
        if (!btn && last_button) {
            led_on = false; // Tắt LED khi nhả
            ESP_LOGI(TAG, "Button RELEASED - LED OFF");
        }
        last_button = btn;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task PWM fade LED
void led_task(void *pvParameter) {
    pwm_init();

    int duty = 0;
    int step = 5;

    while(1) {
        if (led_on) {
            duty += step;
            if (duty >= 255 || duty <= 0) step = -step;
            set_led_brightness(duty);
        } else {
            set_led_brightness(0); // LED tắt
        }
        vTaskDelay(pdMS_TO_TICKS(30)); // tốc độ fade
    }
}

void app_main() {
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);
}
