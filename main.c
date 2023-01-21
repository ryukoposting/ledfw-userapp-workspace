#include "leds.h"

DEFINE_PRIMARY_SLOT(m_hue, "Hue", SD_COLOR_HUE);
DEFINE_PRIMARY_SLOT(m_saturation, "Saturation", SD_COLOR_SATURATION);
DEFINE_PRIMARY_SLOT(m_value, "Intensity", SD_INTENSITY);
DEFINE_PRIMARY_SLOT(m_hue_2, "Hue (Secondary)", SD_COLOR_HUE);
DEFINE_PRIMARY_SLOT(m_saturation_2, "Saturation (Secondary)", SD_COLOR_SATURATION);
DEFINE_PRIMARY_SLOT(m_value_2, "Intensity (Secondary)", SD_INTENSITY);
DEFINE_PRIMARY_SLOT(m_rate, "Pulse Rate (BPM)", SD_UNDEFINED);
DEFINE_PRIMARY_SLOT(m_red, "Red", SD_COLOR_ADD_RED);
DEFINE_PRIMARY_SLOT(m_green, "Green", SD_COLOR_ADD_GREEN);
DEFINE_PRIMARY_SLOT(m_blue, "Blue", SD_COLOR_ADD_BLUE);
DEFINE_PRIMARY_SLOT(m_width, "Width", SD_UNDEFINED);

#define P_BASIC_HSV 0
#define P_BASIC_RGB 1
#define P_WAVEFORM_HSV 2
#define P_CHASER_HSV 3
#define P_CHASER_REVERSE_HSV 4

DEFINE_DMX_PERSONALITY(m_basic_hsv, "Basic HSV",
    DMX_SLOT(m_hue),
    DMX_SLOT(m_saturation),
    DMX_SLOT(m_value)
);

DEFINE_DMX_PERSONALITY(m_basic_rgb, "Basic RGB",
    DMX_SLOT(m_red),
    DMX_SLOT(m_green),
    DMX_SLOT(m_blue)
);

DEFINE_DMX_PERSONALITY(m_waveform_hsv, "Waveform",
    DMX_SLOT(m_hue),
    DMX_SLOT(m_saturation),
    DMX_SLOT(m_value),
    DMX_SLOT(m_rate)
);

DEFINE_DMX_PERSONALITY(m_chaser_hsv, "Chaser",
    DMX_SLOT(m_hue),
    DMX_SLOT(m_saturation),
    DMX_SLOT(m_value),
    DMX_SLOT(m_hue_2),
    DMX_SLOT(m_saturation_2),
    DMX_SLOT(m_value_2),
    DMX_SLOT(m_rate),
    DMX_SLOT(m_width)
);

DEFINE_DMX_PERSONALITY(m_chaser_reverse_hsv, "Reverse Chaser",
    DMX_SLOT(m_hue),
    DMX_SLOT(m_saturation),
    DMX_SLOT(m_value),
    DMX_SLOT(m_hue_2),
    DMX_SLOT(m_saturation_2),
    DMX_SLOT(m_value_2),
    DMX_SLOT(m_rate),
    DMX_SLOT(m_width)
);

DEFINE_VECTBL(
    "Example App",          /* App Name */
    "Example Provider",     /* App Provider */
    NULL,                   /* App ID */
    DMX_PERS(m_basic_hsv),
    DMX_PERS(m_basic_rgb),
    DMX_PERS(m_waveform_hsv),
    DMX_PERS(m_chaser_hsv),
    DMX_PERS(m_chaser_reverse_hsv)
);

static int t = 0;

/* initialize the leds as necessary for your code to work. */
void init(led_chan_t *chan)
{
    chan->color_mode = MODE_RGB;

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
    int personality = update_slot_vals(chan);

    unsigned i = 0;

    if (personality == P_BASIC_HSV) { // Basic HSV
        chan->color_mode = MODE_HSV;
        for (unsigned n = 0; n < chan->n_leds; ++n) {
            chan->buffer[i++] = m_hue.value;
            chan->buffer[i++] = m_saturation.value;
            chan->buffer[i++] = m_value.value;
        }
    } else if (personality == P_BASIC_RGB) { // Basic RGB
        chan->color_mode = MODE_RGB;
        for (unsigned n = 0; n < chan->n_leds; ++n) {
            chan->buffer[i++] = m_red.value;
            chan->buffer[i++] = m_green.value;
            chan->buffer[i++] = m_blue.value;
        }
    } else if (personality == P_WAVEFORM_HSV) { // Waveform HSV
        chan->color_mode = MODE_HSV;
        unsigned const dt = 255u * t * m_rate.value / 60000u;
        for (unsigned n = 0; n < chan->n_leds; ++n) {
            unsigned const theta = n * 255u / chan->n_leds;
            chan->buffer[i++] = m_hue.value;
            chan->buffer[i++] = m_saturation.value;
            chan->buffer[i++] = m_value.value * (int)cycle_sine(theta + dt) / 256;
        }
    } else if (personality == P_CHASER_HSV || personality == P_CHASER_REVERSE_HSV) { // Chaser HSV
        chan->color_mode = MODE_HSV;
        unsigned const width = ((unsigned)m_width.value * (unsigned)chan->n_leds) / 255u;
        unsigned const pos = ((t * (unsigned)m_rate.value * (unsigned)chan->n_leds) / 60000u);
        for (unsigned n = 0; n < chan->n_leds; ++n) {

            int const pri = personality == P_CHASER_HSV
                ? ((pos + n) % chan->n_leds) < width
                : ((pos + chan->n_leds - n) % chan->n_leds) < width;

            if (pri) {
                chan->buffer[i++] = m_hue.value;
                chan->buffer[i++] = m_saturation.value;
                chan->buffer[i++] = m_value.value;
            } else {
                chan->buffer[i++] = m_hue_2.value;
                chan->buffer[i++] = m_saturation_2.value;
                chan->buffer[i++] = m_value_2.value;
            }
        }
    }

    t += chan->refresh_time_msec;
}

/* this function pulls values from the dmx_vals buffer and copies them
   into each slot's `value` field  */
int update_slot_vals(led_chan_t const *chan)
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

    return chan ? chan->dmx_personality_idx : 0;
}
