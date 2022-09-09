#include <cstdint>

enum Knob : uint32_t {
    Osc1Frequency = 1,
    Osc2Frequency = 2,
};

class Knobs {
public:
    Knobs(uint8_t signal_pin, uint8_t s0_pin, uint8_t s1_pin, uint8_t s2_pin, uint8_t s3_pin);

    void init() const;

    [[nodiscard]] uint16_t read(Knob knob) const;

private:
    uint8_t signal_pin;
    uint8_t s0_pin;
    uint8_t s1_pin;
    uint8_t s2_pin;
    uint8_t s3_pin;
};

