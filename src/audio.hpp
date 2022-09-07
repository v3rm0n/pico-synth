#include "pico/audio_i2s.h"

#define SAMPLES_PER_BUFFER 256

typedef int16_t (*buffer_callback)();

class Audio {
public:
    Audio(uint32_t sample_rate, uint8_t pin_data, uint8_t pin_bclk, uint8_t pio_sm = 0, uint8_t dma_ch = 0);

    void init();

    void update(buffer_callback cb);

private:
    audio_format_t format;
    struct audio_i2s_config config;
    struct audio_buffer_pool *producer;
};