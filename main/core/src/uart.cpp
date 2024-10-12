#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"

#define BUF_SIZE (1024)
// #define ECHO_TASK_STACK_SIZE (4096)

void echo_task(void *arg)
{
    // Configure USB SERIAL JTAG
    // usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
    //     .rx_buffer_size = BUF_SIZE,
    //     .tx_buffer_size = BUF_SIZE,
    // };

    usb_serial_jtag_driver_config_t usb_serial_jtag_config;
    usb_serial_jtag_config.rx_buffer_size = BUF_SIZE;
    usb_serial_jtag_config.tx_buffer_size = BUF_SIZE;    

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));
    ESP_LOGI("usb_serial_jtag echo", "USB_SERIAL_JTAG init done");

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    if (data == NULL) {
        ESP_LOGE("usb_serial_jtag echo", "no memory for data");
        return;
    }

    while (1) {

        int len = usb_serial_jtag_read_bytes(data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);

        // Write data back to the USB SERIAL JTAG
        if (len) {
            usb_serial_jtag_write_bytes((const char *) data, len, 20 / portTICK_PERIOD_MS);
            data[len] = '\0';
            ESP_LOG_BUFFER_HEXDUMP("Recv str: ", data, len, ESP_LOG_INFO);
        }
    }
}

