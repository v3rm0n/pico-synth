#include "synth.hpp"

constexpr float pi = 3.14159265358979323846f;

uint32_t prng_xorshift_state = 0x32B71700;

uint32_t prng_xorshift_next() {
    uint32_t x = prng_xorshift_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    prng_xorshift_state = x;
    return x;
}

int32_t prng_normal() {
    // rough approximation of a normal distribution
    uint32_t r0 = prng_xorshift_next();
    uint32_t r1 = prng_xorshift_next();
    uint32_t n = ((r0 & 0xffff) + (r1 & 0xffff) + (r0 >> 16) + (r1 >> 16)) / 2;
    return n - 0xffff;
}

const int16_t sine_waveform[256] = {-32768, -32758, -32729, -32679, -32610, -32522, -32413, -32286, -32138, -31972,
                                    -31786, -31581, -31357, -31114, -30853, -30572, -30274, -29957, -29622, -29269,
                                    -28899, -28511, -28106, -27684, -27246, -26791, -26320, -25833, -25330, -24812,
                                    -24279, -23732, -23170, -22595, -22006, -21403, -20788, -20160, -19520, -18868,
                                    -18205, -17531, -16846, -16151, -15447, -14733, -14010, -13279, -12540, -11793,
                                    -11039, -10279, -9512, -8740, -7962, -7180, -6393, -5602, -4808, -4011, -3212,
                                    -2411, -1608, -804, 0, 804, 1608, 2411, 3212, 4011, 4808, 5602, 6393, 7180, 7962,
                                    8740, 9512, 10279, 11039, 11793, 12540, 13279, 14010, 14733, 15447, 16151, 16846,
                                    17531, 18205, 18868, 19520, 20160, 20788, 21403, 22006, 22595, 23170, 23732, 24279,
                                    24812, 25330, 25833, 26320, 26791, 27246, 27684, 28106, 28511, 28899, 29269, 29622,
                                    29957, 30274, 30572, 30853, 31114, 31357, 31581, 31786, 31972, 32138, 32286, 32413,
                                    32522, 32610, 32679, 32729, 32758, 32767, 32758, 32729, 32679, 32610, 32522, 32413,
                                    32286, 32138, 31972, 31786, 31581, 31357, 31114, 30853, 30572, 30274, 29957, 29622,
                                    29269, 28899, 28511, 28106, 27684, 27246, 26791, 26320, 25833, 25330, 24812, 24279,
                                    23732, 23170, 22595, 22006, 21403, 20788, 20160, 19520, 18868, 18205, 17531, 16846,
                                    16151, 15447, 14733, 14010, 13279, 12540, 11793, 11039, 10279, 9512, 8740, 7962,
                                    7180, 6393, 5602, 4808, 4011, 3212, 2411, 1608, 804, 0, -804, -1608, -2411, -3212,
                                    -4011, -4808, -5602, -6393, -7180, -7962, -8740, -9512, -10279, -11039, -11793,
                                    -12540, -13279, -14010, -14733, -15447, -16151, -16846, -17531, -18205, -18868,
                                    -19520, -20160, -20788, -21403, -22006, -22595, -23170, -23732, -24279, -24812,
                                    -25330, -25833, -26320, -26791, -27246, -27684, -28106, -28511, -28899, -29269,
                                    -29622, -29957, -30274, -30572, -30853, -31114, -31357, -31581, -31786, -31972,
                                    -32138, -32286, -32413, -32522, -32610, -32679, -32729, -32758};


Synth::Synth() = default;

void Synth::init() {
    uint16_t attack_ms = 0;
    uint16_t decay_ms = 1000;
    uint16_t sustain = 0xafff;
    uint16_t release_ms = 1000;

    // OSC1
    channels[0].waveforms = Waveform::SAW;
    channels[0].frequency = 262;
    channels[0].attack_ms = attack_ms;
    channels[0].decay_ms = decay_ms;
    channels[0].sustain = sustain;
    channels[0].release_ms = release_ms;

    // OSC2
    channels[1].waveforms = Waveform::SAW;
    channels[1].frequency = 392;
    channels[1].attack_ms = attack_ms;
    channels[1].decay_ms = decay_ms;
    channels[1].sustain = sustain;
    channels[1].release_ms = release_ms;
}

int16_t Synth::get_audio_frame() {
    int32_t sample = 0;  // used to combine channel output

    for (auto &channel: channels) {
        sample += channel.get_sample();
    }

    // Apply volume
    sample = (int64_t(sample) * int32_t(volume)) >> 16;

    // clip result to 16-bit
    sample = sample <= -0x8000 ? -0x8000 : (sample > 0x7fff ? 0x7fff : sample);
    return sample;
}

void Synth::attack() {
    for (auto &channel: channels) {
        channel.trigger_attack();
    }
}

void Synth::release() {
    for (auto &channel: channels) {
        channel.trigger_release();
    }
}

void Synth::configure_envelope(ADSRParameters parameters) {
    for (auto &channel: channels) {
        channel.attack_ms = parameters.attack_ms;
        channel.release_ms = parameters.release_ms;
        channel.decay_ms = parameters.decay_ms;
        channel.sustain = parameters.sustain;
    }
}

void Synth::set_waveform(int channel, Waveform waveform) {
    channels[channel].waveforms = waveform;
}

void Synth::set_frequency(int channel, uint16_t frequency) {
    channels[channel].frequency = frequency;
}

void AudioChannel::trigger_attack() {
    adsr_frame = 0;
    adsr_phase = ADSRPhase::ATTACK;
    adsr_end_frame = (attack_ms * SAMPLE_RATE) / 1000;
    adsr_step = (int32_t(0xffffff) - int32_t(adsr)) / int32_t(adsr_end_frame);
}

void AudioChannel::trigger_decay() {
    adsr_frame = 0;
    adsr_phase = ADSRPhase::DECAY;
    adsr_end_frame = (decay_ms * SAMPLE_RATE) / 1000;
    adsr_step = (int32_t(sustain << 8) - int32_t(adsr)) / int32_t(adsr_end_frame);
}

void AudioChannel::trigger_sustain() {
    adsr_frame = 0;
    adsr_phase = ADSRPhase::SUSTAIN;
    adsr_end_frame = 0;
    adsr_step = 0;
}

void AudioChannel::trigger_release() {
    adsr_frame = 0;
    adsr_phase = ADSRPhase::RELEASE;
    adsr_end_frame = (release_ms * SAMPLE_RATE) / 1000;
    adsr_step = (int32_t(0) - int32_t(adsr)) / int32_t(adsr_end_frame);
}

void AudioChannel::off() {
    adsr_frame = 0;
    adsr_phase = ADSRPhase::OFF;
    adsr_step = 0;
}

int32_t AudioChannel::get_sample() {

    // increment the waveform position counter. this provides an
    // Q16 fixed point value representing how far through
    // the current waveform we are
    waveform_offset += ((frequency * 256) << 8) / SAMPLE_RATE;

    if (adsr_phase == ADSRPhase::OFF) {
        return 0;
    }

    if ((adsr_frame >= adsr_end_frame) && (adsr_phase != ADSRPhase::SUSTAIN)) {
        switch (adsr_phase) {
            case ADSRPhase::ATTACK:
                trigger_decay();
                break;
            case ADSRPhase::DECAY:
                trigger_sustain();
                break;
            case ADSRPhase::RELEASE:
                off();
                break;
            default:
                break;
        }
    }

    adsr += adsr_step;
    adsr_frame++;

    if (waveform_offset & 0x10000) {
        // if the waveform offset overflows then generate a new
        // random noise sample
        noise = prng_normal();
    }

    waveform_offset &= 0xffff;

    // check if any waveforms are active for this channel
    if (waveforms) {
        uint8_t waveform_count = 0;
        int32_t channel_sample = 0;

        if (waveforms & Waveform::NOISE) {
            channel_sample += noise;
            waveform_count++;
        }

        if (waveforms & Waveform::SAW) {
            channel_sample += (int32_t) waveform_offset - 0x7fff;
            waveform_count++;
        }

        // creates a triangle wave of ^
        if (waveforms & Waveform::TRIANGLE) {
            if (waveform_offset < 0x7fff) { // initial quarter up slope
                channel_sample += int32_t(waveform_offset * 2) - int32_t(0x7fff);
            } else { // final quarter up slope
                channel_sample += int32_t(0x7fff) - ((int32_t(waveform_offset) - int32_t(0x7fff)) * 2);
            }
            waveform_count++;
        }

        if (waveforms & Waveform::SQUARE) {
            channel_sample += (waveform_offset < pulse_width) ? 0x7fff : -0x7fff;
            waveform_count++;
        }

        if (waveforms & Waveform::SINE) {
            // the sine_waveform sample contains 256 samples in
            // total so we'll just use the most significant bits
            // of the current waveform position to index into it
            channel_sample += sine_waveform[waveform_offset >> 8];
            waveform_count++;
        }

        if (waveforms & Waveform::WAVE) {
            channel_sample += wave_buffer[wave_buf_pos];
            if (++wave_buf_pos == 64) {
                wave_buf_pos = 0;
                if (wave_buffer_callback)
                    wave_buffer_callback(*this);
            }
            waveform_count++;
        }

        channel_sample = channel_sample / waveform_count;

        channel_sample = (int64_t(channel_sample) * int32_t(adsr >> 8)) >> 16;

        // apply channel volume
        channel_sample = (int64_t(channel_sample) * int32_t(volume)) >> 16;

        // apply channel filter
        //if (filter_enable) {
        //float filter_epow = 1 - expf(-(1.0f / 22050.0f) * 2.0f * pi * int32_t(filter_cutoff_frequency));
        //channel_sample += (channel_sample - filter_last_sample) * filter_epow;
        //}

        //filter_last_sample = channel_sample;

        return channel_sample;
    }
    return 0;
}
