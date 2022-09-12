#include "synth.hpp"
#include "knobs.hpp"
#include "audio.hpp"
#include "encoder.hpp"
#include "switch.hpp"
#include "pins.hpp"
#include "display.hpp"
#include "tiny3x3a.hpp"
#include "pico/multicore.h"
#include "interrupts.hpp"
#include "iostream"

using namespace std;

encoder::Encoder enc(pio0, 1, {ROTARY_SWITCH_A_PIN, ROTARY_SWITCH_B_PIN}, PIN_UNUSED);
Knobs knobs(ADC_MUX_SIGNAL_PIN, ADC_MUX_S0_PIN, ADC_MUX_S1_PIN, ADC_MUX_S2_PIN, ADC_MUX_S3_PIN);
Audio audio(SAMPLE_RATE, PICO_AUDIO_SDA_PIN, PICO_AUDIO_SCL_PIN);
Synth synth;
Display display(i2c0, SCREEN_SDA_PIN, SCREEN_SCL_PIN);
Interrupts interrupts;

class RotarySwitch : public Switch, public InterruptHandler {
public:
    explicit RotarySwitch(uint8_t switch_pin) : Switch(switch_pin) {}

    void handle_press() override {
        synth.attack();
    }

    void handle_release() override {
        synth.release();
    }

    uint8_t get_pin() override {
        return Switch::get_pin();
    }
};

RotarySwitch rotary(ROTARY_SWITCH_PIN);

int16_t audio_update() {
    return synth.get_audio_frame();
}

void core1_audio() {
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true)
        audio.update(audio_update);
#pragma clang diagnostic pop
}


int main() {
    stdio_init_all();

    cout << "Wait for serial console" << endl;
    sleep_ms(5000);
    cout << "Setup" << endl;

    audio.init();
    synth.init();
    rotary.init();
    knobs.init();
    enc.init();

    display.init();
    display.set_font(&Tiny3x3a2pt7b);
    display.draw_string(0, 10, "PICO SYNTHESIZER", true);

    interrupts.register_switch(&rotary);

    cout << "Setup done!" << endl;

    multicore_launch_core1(core1_audio);

    cout << "Audio core launched" << endl;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        uint16_t result1 = knobs.read(Knob::Osc1Frequency);
        synth.set_frequency(0, result1 / 4);
        uint16_t result2 = knobs.read(Knob::Osc2Frequency);
        synth.set_frequency(1, result2 / 4);
        display.display();
    }
#pragma clang diagnostic pop
}