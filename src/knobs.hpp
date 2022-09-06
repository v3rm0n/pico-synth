#include <stdint.h>

enum Knob : uint32_t
{
    Osc1Frequency = 1,
    Osc2Frequency = 2,
};

void init_knobs(uint8_t signal_pin, uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin);

uint16_t read_knob(Knob knob);