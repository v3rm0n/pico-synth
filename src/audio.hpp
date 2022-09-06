#pragma once
#include "pico/audio_i2s.h"

#define SAMPLES_PER_BUFFER 256

typedef int16_t (*buffer_callback)(void);

struct audio_buffer_pool *init_audio(uint32_t sample_rate, uint8_t pin_data, uint8_t pin_bclk, uint8_t pio_sm = 0, uint8_t dma_ch = 0);

void update_buffer(struct audio_buffer_pool *ap, buffer_callback cb);