#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include <math.h>
#include <inttypes.h> // Для использования макросов формата inttypes
#include <stdio.h>
#include "driver/gpio.h"
#include "led_strip.h"
#include "sdkconfig.h"

//=================================
extern const char *TAG;

#define SIN_PERIOD_MS      1000 // 1 second for a full sine wave cycle
#define SIN_VALUES_COUNT   100  // 100 values per period
#define AMPLITUDE          1000 // Amplitude from 0 to 1000

#define BLINK_GPIO GPIO_NUM_8    // RGB LED 
#define HEARTBEAT_PERIOD_MS 1500 // 1 second for a full heartbeat cycle
#define MAX_BRIGHTNESS      5    // 5 of 255

static uint8_t s_led_state = 1;
static led_strip_handle_t led_strip;

void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number */
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };

    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        //.flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));

    /* Set all LEDs off to clear all pixels */
    led_strip_clear(led_strip);
}
//===============================================================

void blink_led(void)
{
    static uint32_t s_time = 0;
    /* If the addressable LED is enabled */
    if (s_led_state) {
        // Calculate the current time position in the cycle
        float t = (float)(s_time % HEARTBEAT_PERIOD_MS) / HEARTBEAT_PERIOD_MS;

        // Generate heartbeat-like pattern with smooth rise and fall
        float brightness;
        if (t < 0.2) {
            brightness = MAX_BRIGHTNESS * sin(t * M_PI / 0.2); // Smooth rise
        } else if (t < 0.8) {
            brightness = MAX_BRIGHTNESS * sin((0.8 - t) * M_PI / 0.6); // Smooth fall
        } else {
            brightness = 0; // Rest period
        }

        uint8_t r = (uint8_t)round(brightness);

        /* Set the LED pixel using RGB values */
        led_strip_set_pixel(led_strip, 0, r, 0, 0);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LEDs off to clear all pixels */
        led_strip_clear(led_strip);
    }
    s_time += 10; // Increment time
}
//===============================================================

int calc_sine_data(bool send_uart)
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    int32_t sine_value = (int32_t)((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

    if (send_uart) {
        // Log the sine value
        ESP_LOGI(TAG, "data %" PRId32, sine_value);
    }

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    return sine_value;
}
//===============================================================
