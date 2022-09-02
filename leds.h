#ifndef _LEDS_H_
#define _LEDS_H_

#include <stdint.h>
#include <stddef.h>

#ifdef VSCODE
#define _DESC_ARCHCPU 4
#define _DESC_ARCHFLAGS 1
#endif

#ifndef CONCAT
#define CONCAT_(X,Y) X ## Y
#define CONCAT(X,Y) CONCAT_(X,Y)
#endif

#define DMX_SLOT(slot_ident) ((uint32_t)(&slot_ident))
#define DMX_PERS(pers_ident) ((uint32_t)(&pers_ident))

#define DEFINE_PRIMARY_SLOT(slot_ident, slot_name, slot_id) \
    volatile static char const CONCAT(slot_def_name_,slot_ident)[] = (slot_name);\
    volatile static struct slot_info const slot_ident = {\
        .name = (uint32_t)CONCAT(slot_def_name_,slot_ident),\
        .type = ST_PRIMARY,\
        .id = slot_id,\
    };

#define DEFINE_SECONDARY_SLOT() TODO


#define DEFINE_DMX_PERSONALITY(pers_ident, pers_name, ...)\
    volatile static char const CONCAT(pers_def_name_,pers_ident)[] = (pers_name);\
    volatile const uint32_t pers_ident[] = {\
        (uint32_t)CONCAT(pers_def_name_,pers_ident),\
        __VA_ARGS__,\
        (uint32_t)NULL\
    };


/** You must use this macro! Put it at the top of a .c file. */
#define DEFINE_VECTBL(app_name, app_provider, app_id, ...) \
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
        ((uint32_t)_DESC_ARCHCPU) | (((uint32_t)_DESC_ARCHFLAGS) << 16),\
        __VA_ARGS__,\
        (uint32_t)NULL\
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
    /** Current DMX personality. */
    uint8_t const dmx_personality_idx;
};

struct __attribute__ ((packed)) slot_info {
    uint32_t const name;
    uint8_t const type;
    uint16_t const id;
    uint8_t value;
};

enum slot_info_type {
    ST_PRIMARY = 0x00,
    ST_SEC_FINE = 0x01,
    ST_SEC_TIMING = 0x02,
    ST_SEC_SPEED = 0x03,
    ST_SEC_CONTROL = 0x04,
    ST_SEC_INDEX = 0x05,
    ST_SEC_ROTATION = 0x06,
    ST_SEC_INDEX_ROTATE = 0x07,
};

enum slot_info_id {
    SD_INTENSITY = 0x0001,
    SD_INTENSITY_MASTER = 0x0002,

    SD_PAN = 0x0101,
    SD_TILT = 0x0101,

    SD_COLOR_WHEEL = 0x0201,
    SD_COLOR_SUB_CYAN = 0x0202,
    SD_COLOR_SUB_YELLOW = 0x0203,
    SD_COLOR_SUB_MAGENTA = 0x0204,
    SD_COLOR_ADD_RED = 0x0205,
    SD_COLOR_ADD_GREEN = 0x0206,
    SD_COLOR_ADD_BLUE = 0x0207,
    SD_COLOR_CORRECTION = 0x0208,
    SD_COLOR_SCROLL = 0x0209,
    SD_COLOR_SEMAPHORE = 0x0210,
    SD_COLOR_ADD_AMBER = 0x0211,
    SD_COLOR_ADD_WHITE = 0x0212,
    SD_COLOR_ADD_WARM_WHITE = 0x0213,
    SD_COLOR_ADD_COOL_WHITE = 0x0214,
    SD_COLOR_SUB_UV = 0x0215,
    SD_COLOR_HUE = 0x0216,
    SD_COLOR_SATURATION = 0x0217,

    SD_STATIC_GOBO_WHEEL = 0x0301,
    SD_ROTO_GOBO_WHEEL = 0x0302,
    SD_PRISM_WHEEL = 0x0303,
    SD_EFFECTS_WHEEL = 0x0304,

    SD_BEAM_SIZE_IRIS = 0x0401,
    SD_EDGE = 0x0402,
    SD_FROST = 0x0403,
    SD_STROBE = 0x0404,
    SD_ZOOM = 0x0405,
    SD_FRAMING_SHUTTER = 0x0406,
    SD_SHUTTER_ROTATE = 0x0407,
    SD_DOUSER = 0x0408,
    SD_BARN_DOOR = 0x0409,

    SD_LAMP_CONTROL = 0x0501,
    SD_FIXTURE_CONTROL = 0x0502,
    SD_FIXTURE_SPEED = 0x0503,
    SD_MACRO = 0x0504,

    SD_UNDEFINED = 0xFFFF,
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

void update_slot_vals(led_chan_t const *chan);

#ifdef __cplusplus
}
#endif

#endif /* _LEDS_H_ */
