#include <cstdint>

class Switch {
public:
    explicit Switch(uint8_t switch_pin);

    void init();

    bool state();

    bool changed();

private:
    uint8_t switch_pin;
};