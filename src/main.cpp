#include "synth.hpp"
#include "knobs.hpp"
#include "audio.hpp"
#include "encoder.hpp"
#include "switch.hpp"
#include "pins.hpp"
#include "display.hpp"
#include "freeMono9pt7b.hpp"

encoder::Encoder enc(pio0, 1, {ROTARY_SWITCH_A_PIN, ROTARY_SWITCH_B_PIN}, PIN_UNUSED);
Knobs knobs(ADC_MUX_SIGNAL_PIN, ADC_MUX_S0_PIN, ADC_MUX_S1_PIN, ADC_MUX_S2_PIN, ADC_MUX_S3_PIN);
Switch rotary(ROTARY_SWITCH_PIN);
Audio audio(SAMPLE_RATE, PICO_AUDIO_SDA_PIN, PICO_AUDIO_SCL_PIN);
Synth synth;
Display display(i2c0, SCREEN_SDA_PIN, SCREEN_SCL_PIN);

void update_playback() {
    if (rotary.changed()) {
        bool state = rotary.state();
        if (state) {
            synth.attack();
        } else {
            synth.release();
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
    rotary.init();
    knobs.init();
    enc.init();

    display.init();
    display.set_font(&FreeMono9pt7b);
    display.draw_string(0, 30, "Hello", 1);
    display.display();

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        uint16_t result1 = knobs.read(Knob::Osc1Frequency);
        synth.set_frequency(1, result1 / 4);
        uint16_t result2 = knobs.read(Knob::Osc2Frequency);
        synth.set_frequency(1, result2 / 4);
        update_playback();
        audio.update(audio_update);
    }
#pragma clang diagnostic pop
}