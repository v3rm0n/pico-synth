#include "knobs.hpp"
#include "hardware/gpio.h"
#include "hardware/adc.h"

uint8_t _s0_pin, _s1_pin, _s2_pin, _s3_pin;

void init_knobs(uint8_t signal_pin, uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin)
{
    adc_init();
    adc_gpio_init(signal_pin);
    adc_select_input(0);

    gpio_init(s0_pin);
    gpio_init(s1_pin);
    gpio_init(s2_pin);
    gpio_init(s3_pin);

    gpio_set_dir(s0_pin, GPIO_OUT);
    gpio_set_dir(s1_pin, GPIO_OUT);
    gpio_set_dir(s2_pin, GPIO_OUT);
    gpio_set_dir(s3_pin, GPIO_OUT);

    _s0_pin = s0_pin;
    _s1_pin = s1_pin;
    _s2_pin = s2_pin;
    _s3_pin = s3_pin;
}

uint16_t read_knob(Knob knob)
{
    gpio_put(_s0_pin, (knob >> 1) & 1);
    gpio_put(_s1_pin, (knob >> 2) & 1);
    gpio_put(_s2_pin, (knob >> 3) & 1);
    gpio_put(_s3_pin, (knob >> 4) & 1);
    uint16_t result = adc_read();
    return result;
}
