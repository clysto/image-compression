#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>

#include "fifo.h"
#include "img.h"

#define BUFFER_SIZE 1024
uint8_t BUFFER[BUFFER_SIZE] = {0};
uint8_t IMGDATA[256 * 8];

static const struct device *const UART = DEVICE_DT_GET(DT_NODELABEL(uart0));
static const struct device *const LEDS = DEVICE_DT_GET_ANY(gpio_leds);

void uart_send(const uint8_t *data, size_t len) {
    for (int i = 0; i < len; i++) {
        uart_poll_out(UART, data[i]);
    }
}

void uart_recv(uint8_t *data, size_t len) {
    for (int i = 0; i < len; i++) {
        while (uart_poll_in(UART, data + i) < 0)
            ;
    }
}

void uart_fifo_send(FIFO *fifo) {
    unsigned size = FIFO_size(fifo);
    for (int i = 0; i < 4; i++) {
        uart_poll_out(UART, (size >> (8 * i)) & 0xFF);
    }
    while (FIFO_size(fifo) > 0) {
        uart_poll_out(UART, FIFO_readByte(fifo));
    }
}

void encode_image(uint8_t qfactor) {
    uint8_t block[64];
    FIFO fifo;
    Image img;
    FIFO_init(&fifo, BUFFER, BUFFER_SIZE);
    IMG_init(&img, 256, 256, qfactor);
    IMG_encodeHeader(&img, &fifo);
    for (int l = 0; l < 256 / 8; l++) {
        uart_recv(IMGDATA, 256 * 8);
        led_on(LEDS, 1);
        for (int i = 0; i < 8; i += 8) {
            for (int j = 0; j < 256; j += 8) {
                for (int k = 0; k < 8; k++) {
                    memcpy(block + k * 8, &IMGDATA[(i + k) * 256 + j], 8);
                }
                IMG_encodeBlock(&img, block, &fifo);
            }
        }
        if (l == 255 / 8) {
            IMG_encodeComplete(&img, &fifo);
        }
        uart_fifo_send(&fifo);
        led_off(LEDS, 1);
    }
}

void main(void) {
    uint8_t cmd;
    while (true) {
        uart_recv(&cmd, 1);
        switch (cmd) {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
                led_on(LEDS, 0);
                encode_image(cmd);
                led_off(LEDS, 0);
                break;
            default:
                break;
        }
    }
}
