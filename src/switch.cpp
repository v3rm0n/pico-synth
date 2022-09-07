#include "switch.hpp"
#include "hardware/gpio.h"

Switch::Switch(uint8_t switch_pin) {
    this->switch_pin = switch_pin;
}

void Switch::init() {
    gpio_init(switch_pin);
    gpio_set_dir(switch_pin, GPIO_IN);
}

bool Switch::state() {
    return gpio_get(switch_pin);
}

bool Switch::changed() {
    static bool previous_state = false;
    bool current_state = state();
    return current_state != previous_state;
}