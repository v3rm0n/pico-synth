#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"

#include "synth.hpp"
#include "picosynth.hpp"

#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

#define PICO_AUDIO_PACK_I2S_DATA 9
#define PICO_AUDIO_PACK_I2S_BCLK 10

using namespace synth;

synth::AudioChannel synth::channels[CHANNEL_COUNT];

bool __no_inline_not_in_flash_func(get_bootsel_button)() {
    const uint CS_PIN_INDEX = 1;

    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();

    // Set chip select to Hi-Z
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    for (volatile int i = 0; i < 1000; ++i);

    // The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
    // Note the button pulls the pin *low* when pressed.
    bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

    // Need to restore the state of chip select, else we are going to have a
    // bad time when we return to code in flash!
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}

void update_playback(void) {

  static bool bootsel_prev_state = false;

  bool bootsel_state = get_bootsel_button();

  if (bootsel_state == bootsel_prev_state) return;
  bootsel_prev_state = bootsel_state;

  for(uint8_t i = 0; i < 2; i++) {
    if(bootsel_state) {
      channels[i].trigger_attack();
    } else {
      channels[i].trigger_release();
    }
  }
}

int main() {
  stdio_init_all();
  struct audio_buffer_pool *ap = init_audio(synth::sample_rate, PICO_AUDIO_PACK_I2S_DATA, PICO_AUDIO_PACK_I2S_BCLK);

  static uint16_t attack_ms   = 0;
  static uint16_t decay_ms    = 1000;
  static uint16_t sustain     = 0xafff;
  static uint16_t release_ms  = 1000;
  static uint16_t volume      = 10000;

  // OSC1
  channels[0].waveforms   = Waveform::SAW;
  channels[0].frequency = 262;
  channels[0].attack_ms   = attack_ms;
  channels[0].decay_ms    = decay_ms;
  channels[0].sustain     = sustain;
  channels[0].release_ms  = release_ms;
  //channels[0].volume      = volume;

  // OSC2
  channels[1].waveforms   = Waveform::SAW;
  channels[0].frequency = 392;
  channels[1].attack_ms   = attack_ms;
  channels[1].decay_ms    = decay_ms;
  channels[1].sustain     = sustain;
  channels[1].release_ms  = release_ms;
  //channels[1].volume      = volume;

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  while (true) {
    gpio_put(PICO_DEFAULT_LED_PIN, get_bootsel_button() ^ PICO_DEFAULT_LED_PIN_INVERTED);
    update_playback();
    update_buffer(ap, get_audio_frame);
  }

  return 0;
}