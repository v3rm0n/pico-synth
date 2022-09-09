#include "display.hpp"

#define SET_CONTRAST        0x81
#define SET_ENTIRE_ON        0xA4
#define SET_NORM_INV        0xA6
#define SET_DISP            0xAE
#define SET_MEM_ADDR        0x20
#define SET_COL_ADDR        0x21
#define SET_PAGE_ADDR        0x22
#define SET_DISP_START_LINE    0x40
#define SET_SEG_REMAP        0xA0
#define SET_MUX_RATIO        0xA8
#define SET_COM_OUT_DIR        0xC0
#define SET_DISP_OFFSET        0xD3
#define SET_DISP_CLK_DIV    0xD5
#define SET_PRECHARGE        0xD9
#define SET_VCOM_DESEL        0xDB
#define SET_CHARGE_PUMP        0x8D
#define SET_COM_PIN_CFG 0xDA
#define SET_SCROLL 0x2E


Mode Display::get_mode() {
    return mode;
}

void Display::set_mode(Mode m) {
    this->mode = m;
}

void Display::draw_horizontal_line(int16_t x, int16_t y, int16_t length, uint8_t value) {
    for (int16_t p = x; p <= x + length; p++) {
        set_pixel(p, y, value);
    }
}

void Display::draw_vertical_line(int16_t x, int16_t y, int16_t length, uint8_t value) {
    for (int16_t p = y; p <= y + length; p++) {
        set_pixel(x, p, value);
    }
}

void Display::draw_filled_rectangle(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t value) {
    for (int16_t _y = y; _y <= y + height; _y++) {
        draw_horizontal_line(x, _y, width, value);
    }
}

void Display::draw_rectangle(int16_t x, int16_t y, int16_t width, int16_t height, uint8_t value) {
    draw_horizontal_line(x, y, width - 1, value);
    draw_horizontal_line(x, y + height - 1, width, value);
    draw_vertical_line(x, y, height - 1, value);
    draw_vertical_line(x + width - 1, y, height, value);
}

void Display::draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t value) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    for (;;) {
        set_pixel(x0, y0, value);
        if (x0 == x1 && y0 == y1) break;
        int e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

void Display::draw_circle(int16_t center_x, int16_t center_y, int16_t radius, uint8_t value) {
    int16_t x = 0, y, p;

    y = radius;
    p = 3 - (2 * radius);

    while (x < y) {
        set_pixel(center_x + x, center_y + y, value);
        set_pixel(center_x - x, center_y + y, value);
        set_pixel(center_x + x, center_y - y, value);
        set_pixel(center_x - x, center_y - y, value);
        set_pixel(center_x + y, center_y + x, value);
        set_pixel(center_x - y, center_y + x, value);
        set_pixel(center_x + y, center_y - x, value);
        set_pixel(center_x - y, center_y - x, value);

        x++;
        if (p < 0) {
            p = p + 4 * x + 6;
        } else {
            y--;
            p = p + 4 * (x - y) + 10;
        }
        set_pixel(center_x + x, center_y + y, value);
        set_pixel(center_x - x, center_y + y, value);
        set_pixel(center_x + x, center_y - y, value);
        set_pixel(center_x - x, center_y - y, value);
        set_pixel(center_x + y, center_y + x, value);
        set_pixel(center_x - y, center_y + x, value);
        set_pixel(center_x + y, center_y - x, value);
        set_pixel(center_x - y, center_y - x, value);

    }
}

// draw circle, centred on x,y with radius r
void Display::draw_filled_circle(int16_t center_x, int16_t center_y, int16_t radius, uint8_t value) {
    int16_t x = 0, y, p;

    y = radius;
    p = 3 - (2 * radius);

    while (x < y) {
        draw_line(center_x + x, center_y + y, center_x - x, center_y + y, value);
        draw_line(center_x + x, center_y - y, center_x - x, center_y - y, value);
        draw_line(center_x + y, center_y + x, center_x - y, center_y + x, value);
        draw_line(center_x + y, center_y - x, center_x - y, center_y - x, value);

        x++;
        if (p < 0) {
            p = p + 4 * x + 6;
        } else {
            y--;
            p = p + 4 * (x - y) + 10;
        }
        draw_line(center_x + x, center_y + y, center_x - x, center_y + y, value);
        draw_line(center_x + x, center_y - y, center_x - x, center_y - y, value);
        draw_line(center_x + y, center_y + x, center_x - y, center_y + x, value);
        draw_line(center_x + y, center_y - x, center_x - y, center_y - x, value);

    }
}

void Display::set_font(const GFXfont *f) {
    this->font = (GFXfont *) f;
}

int16_t Display::draw_glyph(int16_t x, int16_t y, char character, uint8_t value) {
    if (font == nullptr) {
        // if we don't have a font specified then abort
        return 0;
    }

    // we have a font, next we need to make sure it holds the ASCII character specified
    if ((character >= font->first) && (character <= font->last)) {
        // the required character is present in the font
        // offset is the ASCII character, less the first character in the font
        GFXglyph *glyph = font->glyph + character - font->first;
        uint16_t baseOffset = glyph->bitmap_offset;        // offset into the font bitmap array
        uint8_t w = glyph->width;
        uint8_t h = glyph->height;
        int8_t xo = glyph->x_offset;
        int8_t yo = glyph->y_offset;
        uint8_t xx, yy, bits = 0, bit = 0;

        for (yy = 0; yy < h; yy++) {
            for (xx = 0; xx < w; xx++) {
                if (!(bit++ & 7)) {
                    bits = font->bitmap[baseOffset++];
                }
                if (bits & 0x80) {
                    set_pixel(x + xo + xx, y + font->y_advance + yo + yy, value);
                }
                bits <<= 1;
            }
        }

        return glyph->x_advance;
    } else {
        return 0;
    }
}

int16_t Display::draw_string(int16_t x, int16_t y, const char character[], uint8_t value) {
    uint8_t idx = 0;
    char C = character[idx];
    int16_t _x = x;

    while (C != '\0') {
        _x += draw_glyph(_x, y - font->y_advance, C, value);
        idx++;
        C = character[idx];
    }

    return _x;
}

void Display::clear_clip() {
    clip_rect.x = 0;
    clip_rect.y = 0;
    clip_rect.width = DISPLAY_WIDTH - 1;
    clip_rect.height = DISPLAY_HEIGHT - 1;
}

void Display::set_clip(int16_t x, int16_t y, uint8_t clip_width, uint8_t clip_height) {
    clip_rect.x = x;
    clip_rect.y = y;
    clip_rect.width = clip_width;
    clip_rect.height = clip_height;
}

void Display::set_clip(Rectangle *rectangle) {
    clip_rect.x = rectangle->x;
    clip_rect.y = rectangle->y;
    clip_rect.width = rectangle->width;
    clip_rect.height = rectangle->height;
}

void Display::get_clip(Rectangle *rectangle) {
    rectangle->x = clip_rect.x;
    rectangle->y = clip_rect.y;
    rectangle->width = clip_rect.width;
    rectangle->height = clip_rect.height;
}

void Display::clear() {
    Mode old_mode = get_mode();
    set_mode(Set);
    for (int16_t x = clip_rect.x; x <= (clip_rect.x + clip_rect.width); x++) {
        for (int16_t y = clip_rect.y; y <= (clip_rect.y + clip_rect.height); y++) {
            set_pixel(x, y, 0);
        }
    }
    set_mode(old_mode);
}

uint8_t Display::get_pixel(int16_t x, int16_t y) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return 0;
    }

    uint8_t page = y / PAGE_HEIGHT;
    uint8_t *ptr = display_buffer + x * PAGE_HEIGHT + page + 1;

    uint8_t current_byte = *ptr;
    uint8_t current_bit = (current_byte >> (y % PAGE_HEIGHT)) & 0x01;

    return current_bit;
}

uint8_t Display::set_pixel(int16_t x, int16_t y, uint8_t value) {
    if (x < 0 || x > width || y < 0 || y >= height || x < clip_rect.x || x > (clip_rect.x + clip_rect.width) ||
        y < clip_rect.y || y > (clip_rect.y + clip_rect.height)) {
        return 0;
    }

    uint8_t page = y / PAGE_HEIGHT;
    uint8_t bit = 1 << (y % PAGE_HEIGHT);
    uint8_t *ptr = display_buffer + x * PAGE_HEIGHT + page + 1;

    uint8_t current_byte = *ptr;
    uint8_t current_bit = (current_byte >> (y % PAGE_HEIGHT)) & 0x01;

    switch (mode) {
        case Set: {
            break;
        }
        case Or: {
            value |= get_pixel(x, y);
            break;
        }
        case And: {
            value &= get_pixel(x, y);
            break;
        }
        case Xor: {
            value ^= get_pixel(x, y);
            break;
        }
        case Not: {
            value = !value;
        }
    }

    if (value == 0) {
        *ptr &= ~bit;
    } else if (value == 1) {
        *ptr |= bit;
    }

    return current_bit;
}

void Display::init() {
    i2c_init(i2c, 400 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    // initialise the display
    const uint8_t init_sequence[] = {
            SET_DISP | 0x00,
            SET_MEM_ADDR,
            0x00,
            SET_DISP_START_LINE | 0x00,
            SET_SEG_REMAP | 0x01,
            SET_MUX_RATIO,
            DISPLAY_HEIGHT - 1,
            SET_COM_OUT_DIR | 0x08,
            SET_DISP_OFFSET,
            0x00,
            SET_COM_PIN_CFG,
            0x02,
            SET_DISP_CLK_DIV,
            0x80,
            SET_PRECHARGE,
            0xF1,
            SET_VCOM_DESEL,
            0x30,
            SET_CONTRAST,
            0xFF,
            SET_ENTIRE_ON,
            SET_NORM_INV,
            SET_CHARGE_PUMP,
            0x14,
            SET_SCROLL | 0x00,
            SET_DISP | 0x01
    };

    for (unsigned char i: init_sequence) {
        write_cmd(i);
    }

    display_buffer[0] = 0x40;
    for (uint16_t i = 1; i < display_buffer_size; i++) {
        display_buffer[i] = 0b00000000;
    }

    display();
}

void Display::display() {
    write_cmd(SET_MEM_ADDR);
    write_cmd(0b01);

    write_cmd(SET_COL_ADDR);
    write_cmd(0);
    write_cmd(DISPLAY_WIDTH - 1);

    write_cmd(SET_PAGE_ADDR);
    write_cmd(0);
    write_cmd(PAGE_HEIGHT - 1);

    i2c_write_blocking(i2c, i2c_address, display_buffer, display_buffer_size, false);
}

void Display::clear_all() {
    for (uint16_t i = 1; i < display_buffer_size; i++) {
        display_buffer[i] = 0x00;
    }
}

void Display::write_cmd(uint8_t cmd) {
    uint8_t command_buffer[2];

    command_buffer[0] = 0x80;
    command_buffer[1] = cmd;

    i2c_write_blocking(i2c, i2c_address, command_buffer, 2, false);
}

Display::Display(i2c_inst_t *i2c, uint8_t sda_pin, uint8_t scl_pin) {
    this->i2c = i2c;
    this->sda_pin = sda_pin;
    this->scl_pin = scl_pin;
}
