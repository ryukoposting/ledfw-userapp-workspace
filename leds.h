#ifndef _LEDS_H_
#define _LEDS_H_

#include <stdint.h>
#include <stddef.h>

/** You must use this macro! Put it at the top of a .c file. */
#define DEFINE_LED_VECTBL(app_name, app_provider, app_id) \
    volatile static char const vectbl_app_name_[] = (app_name);\
    volatile static char const vectbl_app_provider_[] = (app_provider);\
    volatile const uint32_t led_vectbl_[] \
    __attribute__((section(".functab"))) = { \
        0x00041198,\
        0x00010000,\
        (uint32_t)init,\
        (uint32_t)refresh,\
        (uint32_t)vectbl_app_name_,\
        (uint32_t)vectbl_app_provider_,\
        (uint32_t)app_id,\
        ((uint32_t)_DESC_ARCHCPU) | (((uint32_t)_DESC_ARCHFLAGS) << 16)\
    };

#define MODE_RGB (0)
#define MODE_HSV (1)
#define MODE_HSL (2)

#ifdef __cplusplus
extern "C" {
#endif

struct __attribute__ ((packed)) led_chan {
    /** The buffer containing data for each LED. The buffer's length is (3 * `n_leds`) */
    uint8_t *buffer;
    /** A buffer containing current DMX channel values. The length of this buffer is given by `dmx_vals_len`. */
    uint8_t const *dmx_vals;
    /** The channel ID - for example, `chan=0` for channel 0. */
    uint8_t const id;
    /** The current color mode. 0=RGB, 1=HSV, 2=HSL. */
    uint8_t color_mode;
    /** The time interval between calls to `refresh`. Measured in milliseconds. */
    uint16_t const refresh_time_msec;
    /** The number of LEDs in the strip. */
    uint16_t const n_leds;
    /** The length of the `dmx_vals` array. */
    uint16_t const dmx_vals_len;
};

/** Contains information about an LED channel. */
typedef struct led_chan led_chan_t;

extern volatile const uint32_t led_vectbl_[] __attribute__((section(".functab")));

void init(led_chan_t *channel);
void refresh(led_chan_t *channel);

/** Transforms a 0-255 range into a single, full cycle of a sine wave.
 *  
 * Approximates the following function:
 *    f(x) = 127.5 + (127.5 * sin(2 * pi * x / 256))
 * 
 * See the function here: https://www.desmos.com/calculator/lsfjr1dhup
 */
static inline uint8_t cycle_sine(uint8_t x)
{
    if (x <= 25)
        return 127 + (x * 3);
    else if (x <= 46)
        return 204 + ((x - 26) * 2);
    else if (x <= 62)
        return 244 + ((x - 47) * 3 / 4);
    else if (x <= 65)
        return 255;
    else if (x <= 82)
        return 255 - ((x - 66) * 3 / 4);
    else if (x <= 102)
        return 241 - ((x - 83) * 2);
    else if (x <= 145)
        return 202 - ((x - 103) * 3);
    else if (x <= 180)
        return 72 - ((x - 146) * 2);
    else if (x <= 187)
        return 5 - ((x - 181) * 3 / 4);
    else if (x <= 193)
        return 0;
    else if (x <= 210)
        return (x - 194) * 3 / 4;
    else if (x <= 231)
        return 13 + ((x - 211) * 2);
    else
        return 56 + ((x - 232) * 3);
}

/** Cosine easing function.
 * 
 * approximates the following function:
 *     f(x) = 127.5 - (127.5 * cos(pi * x / 256))
 * 
 * See the easing function here: https://www.desmos.com/calculator/x8z8ztxtga
 */
static inline uint8_t ease_cosine(uint8_t x)
{
    if (x <= 5)
        return 0;
    else if (x <= 28)
        return (x - 6) / 3;
    else if (x <= 64)
        return 7 + ((x - 29) * 5 / 6);
    else if (x <= 99)
        return 38 + ((x - 65) * 4 / 3);
    else if (x <= 154)
        return 85 + ((x - 99) * 3 / 2);
    else if (x <= 203)
        return 168 + ((x - 153) * 5 / 4);
    else if (x <= 226)
        return 231 + ((x - 204) * 3 / 4);
    else if (x <= 251)
        return 247 + ((x - 227) / 3);
    else
        return 255;
}

static inline uint32_t ceil_add_u32(uint32_t x, uint32_t y)
{
    return (x > UINT32_MAX - y) ? UINT32_MAX : x + y;
}

static inline uint32_t floor_sub_u32(uint32_t x, uint32_t y)
{
    return (x < y) ? 0 : x - y;
}

static inline unsigned abs(int x)
{
    return (x < 0) ? -x : x;
}

static inline int circular_distance(int x, int y, int max)
{
    int dist = abs(x - y);
    if (dist > max / 2)
        return max - dist;
    else
        return dist;
}

#ifdef __cplusplus
}
#endif

#endif /* _LEDS_H_ */