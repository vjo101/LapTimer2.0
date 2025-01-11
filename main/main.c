
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
// output
#define OUTPUT_GPIO GPIO_NUM_23
// set this to whatever pin is being used
#define INPUT_GPIO GPIO_NUM_25
// pin used for led
#define BLINK_GPIO 2
// for blinking led
static uint8_t led_state = 0;

#define ESP_INTR_FLAG_DEFAULT 0

static void IRAM_ATTR handler(void *arg) {
    printf("rising edge detected\n");
    gpio_set_level(BLINK_GPIO, led_state);
    led_state = !led_state;
}

static void setup(void)
{
     //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL<<OUTPUT_GPIO;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1ULL<<INPUT_GPIO;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // // create pin to constanly outpu high
    // gpio_reset_pin(OUTPUT_GPIO);
    // gpio_set_direction(OUTPUT_GPIO, GPIO_MODE_OUTPUT);
    // // set to high
    // gpio_pullup_en(OUTPUT_GPIO);
    // gpio_set_intr_type(OUTPUT_GPIO, GPIO_INTR_DISABLE);

    // gpio_reset_pin(INPUT_GPIO);
    // /* Set the GPIO as a push/pull output */
    // gpio_input_enable(INPUT_GPIO);
    // gpio_set_direction(INPUT_GPIO, GPIO_MODE_INPUT);
    // // this makes sure it only interrupts on rising edges, so when the input changes from LOW to HIGH (ie when laser is first broken).
    // gpio_set_intr_type(INPUT_GPIO, GPIO_INTR_POSEDGE);
    
}

static void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}



void app_main(void)
{
    
    // configure the GPIO pin to be used as an interupt
    setup();
    printf("testinh\n");
    // configure led
    configure_led();
    // set led to on initially
    gpio_set_level(BLINK_GPIO, led_state);
    printf("light should be on\n");
    // install the interupt service. I dont know what this does
    gpio_install_isr_service(0);
    // add the function to the interupt
    gpio_isr_handler_add(INPUT_GPIO, handler, NULL);

}