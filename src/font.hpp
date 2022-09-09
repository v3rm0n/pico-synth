#include "pico/stdlib.h"

typedef struct {
    uint16_t bitmap_offset;
    uint8_t width;
    uint8_t height;
    uint8_t x_advance;
    int8_t x_offset;
    int8_t y_offset;
} GFXglyph;

typedef struct {
    uint8_t *bitmap;
    GFXglyph *glyph;
    uint16_t first;
    uint16_t last;
    uint8_t y_advance;
} GFXfont;