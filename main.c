#include "leds.h"

DEFINE_PRIMARY_SLOT(m_hue, "Hue", SD_COLOR_HUE);
DEFINE_PRIMARY_SLOT(m_saturation, "Saturation", SD_COLOR_SATURATION);
DEFINE_PRIMARY_SLOT(m_value, "Intensity", SD_INTENSITY);
DEFINE_PRIMARY_SLOT(m_rate, "Pulse Rate (BPM)", SD_UNDEFINED);

DEFINE_DMX_PERSONALITY(m_default_pers, "Default Personality",
    DMX_SLOT(m_hue),
    DMX_SLOT(m_saturation),
    DMX_SLOT(m_value),
    DMX_SLOT(m_rate)
);

DEFINE_VECTBL(
    "Example App",          /* App Name */
    "Example Provider",     /* App Provider */
    NULL,                   /* App ID */
    DMX_PERS(m_default_pers)
);

static int t = 0;

/* initialize the leds as necessary for your code to work. */
void init(led_chan_t *chan)
{
    chan->color_mode = MODE_HSV;

    uint32_t i = 0;
    for (uint16_t n = 0; n < chan->n_leds; n++) {
        chan->buffer[i++] = 0; /* set red to 0 */
        chan->buffer[i++] = 0; /* set green to 0 */
        chan->buffer[i++] = 0; /* set blue to 0 */
    }
}

/* this function is called every `refresh_period_msec` milliseconds.
 * changing the values in `chan->buffer` will change the colors of the LEDs. */
void refresh(led_chan_t *chan)
{
    update_slot_vals(chan);

    unsigned i = 0;
    for (unsigned n = 0; n < chan->n_leds; ++n) {
        unsigned const theta = n * 255u / chan->n_leds;
        unsigned const dt = 255u * t * m_rate.value / 60000u;
        chan->buffer[i++] = m_hue.value;
        chan->buffer[i++] = m_saturation.value;
        chan->buffer[i++] = m_value.value * (int)cycle_sine(theta + dt) / 256;
    }

    t += chan->refresh_time_msec;
}

void update_slot_vals(led_chan_t const *chan)
{
    const size_t max_dmx_personality = (sizeof(led_vectbl_) / sizeof(led_vectbl_[0])) - 9;

    if (chan && chan->dmx_personality_idx < max_dmx_personality) {
        uint32_t const *dmx_pers = (uint32_t const*)led_vectbl_[chan->dmx_personality_idx + 8];
        if (dmx_pers) {
            for (size_t i = 1; chan->dmx_vals && dmx_pers[i] && i <= chan->dmx_vals_len; ++i) {
                struct slot_info *slot = (struct slot_info*)dmx_pers[i];
                slot->value = chan->dmx_vals[i-1];
            }
        }
    }
}
