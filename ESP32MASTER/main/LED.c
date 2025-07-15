#include "LED.h"

void ledOnOFF(uint8_t state)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(13, state);
}

void configure_led(void)
{
    gpio_reset_pin(13);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(13, GPIO_MODE_OUTPUT);
}
