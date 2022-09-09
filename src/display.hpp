#include "pico/stdlib.h"
#include <cstdlib>
#include "hardware/i2c.h"

#include "font.hpp"

#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH 128
#define PAGE_HEIGHT 8
#define I2C_ADDRESS 0x3C

#define PROGMEM

enum Mode : uint8_t {
    Set = 0,
    Or = 1,
    And = 2,
    Xor = 3,
    Not = 4,
};

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t width;
    uint8_t height;
} Rectangle;

class Display {
public:
    Display(i2c_inst_t *i2c, uint8_t sda_pin, uint8_t scl_pin);

    Mode get_mode();

    void set_mode(Mode mode);

    uint8_t get_pixel(int16_t x, int16_t y);

    uint8_t set_pixel(int16_t x, int16_t y, uint8_t value);

    void init();

    void display();

    void clear_all();

    void clear();

    void clear_clip();

    void set_clip(int16_t x, int16_t y, uint8_t width, uint8_t height);

    void set_clip(Rectangle *rectangle);

    void get_clip(Rectangle *rectangle);

    void draw_horizontal_line(int16_t x, int16_t y, int16_t length, uint8_t value);

    void draw_vertical_line(int16_t x, int16_t y, int16_t length, uint8_t value);

    void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t value);

    void draw_filled_rectangle(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t value);

    void draw_rectangle(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t value);

    void draw_circle(int16_t center_x, int16_t center_y, int16_t radius, uint8_t value);

    void draw_filled_circle(int16_t center_x, int16_t center_y, int16_t radius, uint8_t value);

    void set_font(const GFXfont *font);

    int16_t draw_glyph(int16_t x, int16_t y, char character, uint8_t value);

    int16_t draw_string(int16_t x, int16_t y, const char character[], uint8_t value);

private:
    Mode mode = Mode::Set;

    Rectangle clip_rect = {0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1};

    GFXfont *font = nullptr;

    i2c_inst_t *i2c = nullptr;
    uint sda_pin;
    uint scl_pin;
    uint8_t i2c_address = I2C_ADDRESS;
    uint8_t height = DISPLAY_HEIGHT;
    uint8_t width = DISPLAY_WIDTH;

    uint16_t display_buffer_size = DISPLAY_WIDTH * PAGE_HEIGHT + 1;
    uint8_t *display_buffer = static_cast<uint8_t *>(malloc(display_buffer_size));

    void write_cmd(uint8_t cmd);
};