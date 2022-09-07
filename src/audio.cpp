#include "audio.hpp"

Audio::Audio(uint32_t sample_rate, uint8_t data_pin, uint8_t bclk_pin, uint8_t pio_sm, uint8_t dma_ch) {
    audio_format_t audio_format = {
            .sample_freq = sample_rate,
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .channel_count = 1,
    };
    this->format = audio_format;

    struct audio_i2s_config audio_config = {
            .data_pin = data_pin,
            .clock_pin_base = bclk_pin,
            .dma_channel = dma_ch,
            .pio_sm = pio_sm,
    };
    this->config = audio_config;

    this->producer = nullptr;
}

void Audio::init() {

    static struct audio_buffer_format producer_format = {
            .format = &format,
            .sample_stride = 2};

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(
            &producer_format,
            3,
            SAMPLES_PER_BUFFER);

    const struct audio_format *output_format;

    output_format = audio_i2s_setup(&format, &config);
    if (!output_format) {
        panic("Unable to open audio device.\n");
    }

    bool status = audio_i2s_connect(producer_pool);
    if (!status) {
        panic("Unable to connect to audio device.\n");
    }

    audio_i2s_set_enabled(true);

    this->producer = producer_pool;
}

void Audio::update(buffer_callback cb) {
    struct audio_buffer *buffer = take_audio_buffer(producer, true);
    auto *samples = reinterpret_cast<int16_t *>(buffer->buffer->bytes);
    for (uint i = 0; i < buffer->max_sample_count; i++) {
        samples[i] = cb();
    }
    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(producer, buffer);
}
