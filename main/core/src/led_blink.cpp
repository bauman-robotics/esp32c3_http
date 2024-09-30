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

#include "variables.h"
#include "main.h"
//=================================
extern const char *TAG;


#define BLINK_GPIO GPIO_NUM_8    // RGB LED 
#define HEARTBEAT_PERIOD_MS 1500 // 1 second for a full heartbeat cycle
#define MAX_BRIGHTNESS      5    // 5 of 255

static uint8_t s_led_state = 1;
static led_strip_handle_t led_strip;

void configure_led(void);
void blink_led(void);

//=================================

void configure_led(void)
{    
    var.leds.red = 1;
    var.leds.green = 0; 
    var.leds.blue = 0;

    var.leds.flags = LEDS_NO_CONNECT_STATE;

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

        uint8_t red_value = 0;
        uint8_t green_value = 1;
        uint8_t blue_value = 0;

        switch (var.leds.flags) {
            case LEDS_NO_CONNECT_STATE:

                red_value = MAX_BRIGHTNESS;
                green_value = 0;
                blue_value = 0;
                
                break;
            case LEDS_GOT_IP_STATE:
                red_value = 0;
                green_value = MAX_BRIGHTNESS;
                blue_value = 0;

                break;
            case LEDS_CONNECT_TO_SERVER_STATE:
                // Умножаем булевые значения на r и преобразуем в uint8_t
                red_value   = (uint8_t)(var.leds.red   * r);
                green_value = (uint8_t)(var.leds.green * r);
                blue_value  = (uint8_t)(var.leds.blue  * r);

                break; 
        }

        /* Set the LED pixel using RGB values */
        //led_strip_set_pixel(led_strip, 0, r, 0, 0);

        led_strip_set_pixel(led_strip, 0, green_value,  red_value, blue_value );
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LEDs off to clear all pixels */
        led_strip_clear(led_strip);
    }
    s_time += 10; // Increment time
}
//===============================================================


