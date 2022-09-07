#include "synth.hpp"
#include "knobs.hpp"
#include "audio.hpp"
#include "screen.hpp"
#include "encoder.hpp"
#include "switch.hpp"

#define PICO_AUDIO_PACK_I2S_DATA 9
#define PICO_AUDIO_PACK_I2S_BCLK 10

#define ROTARY_SWITCH_B_PIN 14
#define ROTARY_SWITCH_A_PIN 13
#define ROTARY_SWITCH_PIN 15

#define SCREEN_SCL_PIN 16
#define SCREEN_SCA_PIN 17

#define ADC_MUX_S3_PIN 18
#define ADC_MUX_S2_PIN 19
#define ADC_MUX_S1_PIN 20
#define ADC_MUX_S0_PIN 21

#define ADC_MUX_SIGNAL_PIN 26


encoder::Encoder enc(pio0, 1, {ROTARY_SWITCH_A_PIN, ROTARY_SWITCH_B_PIN}, PIN_UNUSED);
Knobs knobs(ADC_MUX_SIGNAL_PIN, ADC_MUX_S0_PIN, ADC_MUX_S1_PIN, ADC_MUX_S2_PIN, ADC_MUX_S3_PIN);
Switch rotary(ROTARY_SWITCH_PIN);
Audio audio(SAMPLE_RATE, PICO_AUDIO_PACK_I2S_DATA, PICO_AUDIO_PACK_I2S_BCLK);
Screen screen(SCREEN_SCL_PIN, SCREEN_SCA_PIN);
Synth synth;

void update_playback() {
    if (rotary.changed()) {
        for (uint8_t i = 0; i < 2; i++) {
            if (rotary.state()) {
                synth.attack();
            } else {
                synth.release();
            }
        }
    }
}

int16_t audio_update() {
    return synth.get_audio_frame();
}

int main() {
    stdio_init_all();

    audio.init();
    synth.init();
    screen.init();
    rotary.init();
    knobs.init();
    enc.init();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        long delta = enc.delta();
        if (delta != 0) {
            if (delta > 0)
                screen.scroll_left();
            else
                screen.scroll_right();
        }
        uint16_t result1 = knobs.read(Knob::Osc1Frequency);
        synth.set_frequency(1, result1 / 4);
        uint16_t result2 = knobs.read(Knob::Osc2Frequency);
        synth.set_frequency(1, result2 / 4);
        update_playback();
        audio.update(audio_update);
    }
#pragma clang diagnostic pop
}