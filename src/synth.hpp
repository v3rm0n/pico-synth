#pragma once

#include <cstdint>
#include <functional>

#define CHANNEL_COUNT 2
#define SAMPLE_RATE 44100

enum Waveform {
    NOISE = 128,
    SQUARE = 64,
    SAW = 32,
    TRIANGLE = 16,
    SINE = 8,
    WAVE = 1
};

enum class ADSRPhase : uint8_t {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE,
    OFF
};

class AudioChannel {
public:
    void trigger_attack();

    void trigger_decay();

    void trigger_sustain();

    void trigger_release();

    void off();

private:
    uint8_t waveforms = 0;    // bitmask for enabled waveforms (see AudioWaveform enum for values)
    uint16_t frequency = 660; // frequency of the voice (Hz)
    uint16_t volume = 0xffff; // channel volume (default 50%)

    uint16_t attack_ms = 2;        // attack period
    uint16_t decay_ms = 6;         // decay period
    uint16_t sustain = 0xffff;     // sustain volume
    uint16_t release_ms = 1;       // release period
    uint16_t pulse_width = 0x7fff; // duty cycle of square wave (default 50%)
    int16_t noise = 0;             // current noise value

    uint32_t waveform_offset = 0; // voice offset (Q8)

    uint32_t adsr_frame = 0;     // number of frames into the current ADSR phase
    uint32_t adsr_end_frame = 0; // frame target at which the ADSR changes to the next phase
    uint32_t adsr = 0;
    int32_t adsr_step = 0;
    ADSRPhase adsr_phase = ADSRPhase::OFF;

    uint8_t wave_buf_pos = 0; //
    int16_t wave_buffer[64];  // buffer for arbitrary waveforms. small as it's filled by user callback

    void (*wave_buffer_callback)(AudioChannel &channel);

    int32_t get_sample();

    friend class Synth;
};

struct ADSRParameters {
    uint16_t attack_ms;
    uint16_t decay_ms;
    uint16_t sustain;
    uint16_t release_ms;
};

class Synth {
public:
    explicit Synth();

    void init();

    int16_t get_audio_frame();

    void attack();

    void release();

    void configure_envelope(ADSRParameters parameters);

    void set_waveform(int channel, Waveform waveform);

    void set_frequency(int channel, uint16_t frequency);

private:

    uint16_t volume = 0xffff;
    AudioChannel channels[CHANNEL_COUNT];
};
