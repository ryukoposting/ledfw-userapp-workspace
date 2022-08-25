#include "leds.h"

DEFINE_LED_VECTBL("Example App", "Example Provider", NULL) /* this line is required! */

static int t = 0;

#define MAX_BRIGHTNESS 10
#define BPM            120

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
    unsigned i = 0;
    for (unsigned n = 0; n < chan->n_leds; ++n) {
        unsigned const theta = n * 255u / chan->n_leds;
        unsigned const dt = 255u * t * BPM / 60000u;
        chan->buffer[i++] = 180;
        chan->buffer[i++] = 255;
        chan->buffer[i++] = MAX_BRIGHTNESS * (int)cycle_sine(theta + dt) / 256;
    }

    t += chan->refresh_time_msec;
}
