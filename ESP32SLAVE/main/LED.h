#ifndef LED_H
#define LED_H

#include "driver/gpio.h"
#include <stdio.h>

// LED GPIO pin definition
#define LED_GPIO_PIN GPIO_NUM_7

// Function declarations
void ledOnOFF(uint8_t state);
void configure_led(void);

#endif // LED_H
