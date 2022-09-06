#include <math.h>
#include <iostream>

#include "pico/stdlib.h"

#include "synth.hpp"
#include "knobs.hpp"
#include "audio.hpp"

#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/adc.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

#define PICO_AUDIO_PACK_I2S_DATA 9
#define PICO_AUDIO_PACK_I2S_BCLK 10

#define ADC_MUX_SIGNAL_PIN 26
#define ADC_MUX_S0_PIN 21
#define ADC_MUX_S1_PIN 20
#define ADC_MUX_S2_PIN 19
#define ADC_MUX_S3_PIN 18

#define ROTARY_SWITCH_PIN 15
#define ROTARY_SWITCH_A_PIN 14
#define ROTARY_SWITCH_B_PIN 13

synth::AudioChannel synth::channels[CHANNEL_COUNT];

void init_synth()
{
  static uint16_t attack_ms = 0;
  static uint16_t decay_ms = 1000;
  static uint16_t sustain = 0xafff;
  static uint16_t release_ms = 1000;
  static uint16_t volume = 10000;

  // OSC1
  synth::channels[0].waveforms = synth::Waveform::SAW;
  synth::channels[0].frequency = 262;
  synth::channels[0].attack_ms = attack_ms;
  synth::channels[0].decay_ms = decay_ms;
  synth::channels[0].sustain = sustain;
  synth::channels[0].release_ms = release_ms;
  // channels[0].volume      = volume;

  // OSC2
  synth::channels[1].waveforms = synth::Waveform::SAW;
  synth::channels[1].frequency = 392;
  synth::channels[1].attack_ms = attack_ms;
  synth::channels[1].decay_ms = decay_ms;
  synth::channels[1].sustain = sustain;
  synth::channels[1].release_ms = release_ms;
  // channels[1].volume      = volume;
}

void update_playback(void)
{

  static bool rotary_switch_prev_state = false;

  bool rotary_switch_state = gpio_get(ROTARY_SWITCH_PIN);

  if (rotary_switch_state == rotary_switch_prev_state)
    return;
  rotary_switch_prev_state = rotary_switch_state;

  for (uint8_t i = 0; i < 2; i++)
  {
    if (rotary_switch_state)
    {
      synth::channels[i].trigger_attack();
    }
    else
    {
      synth::channels[i].trigger_release();
    }
  }
}

int main()
{
  stdio_init_all();
  struct audio_buffer_pool *ap = init_audio(synth::sample_rate, PICO_AUDIO_PACK_I2S_DATA, PICO_AUDIO_PACK_I2S_BCLK);

  init_synth();

  gpio_init(ROTARY_SWITCH_PIN);
  gpio_set_dir(ROTARY_SWITCH_PIN, GPIO_IN);

  init_knobs(ADC_MUX_SIGNAL_PIN, ADC_MUX_S0_PIN, ADC_MUX_S1_PIN, ADC_MUX_S2_PIN, ADC_MUX_S3_PIN);

  while (true)
  {
    uint16_t result1 = read_knob(Knob::Osc1Frequency);
    synth::channels[0].frequency = result1 / 4;
    uint16_t result2 = read_knob(Knob::Osc2Frequency);
    synth::channels[1].frequency = result2 / 4;
    update_playback();
    update_buffer(ap, synth::get_audio_frame);
  }

  return 0;
}