#include <cstdint>

class Screen {
public:
    Screen(uint8_t scl_pin, uint8_t sda_pin);

    void init() const;

    void scroll_left();

    void scroll_right();

private:
    uint8_t scl_pin;
    uint8_t sda_pin;
};