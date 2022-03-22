#include "sidlib.h"
#include "gameboy.h"
#include <stdint.h>
#include <sys/time.h>
#include "error.h"

// Key press bits
#define MY_KEY_UP_BIT 0x01
#define MY_KEY_DOWN_BIT 0x02
#define MY_KEY_RIGHT_BIT 0x04
#define MY_KEY_LEFT_BIT 0x08
#define MY_KEY_A_BIT 0x10
#define MY_KEY_B_BIT 0x20
#define MY_KEY_SELECT_BIT 0x40
#define MY_KEY_START_BIT 0x80

gameboy_t gameboy;
struct timeval start;
struct timeval paused;


// ======================================================================
static void set_grey(guchar *pixels, int row, int col, int width, guchar grey)
{
    const size_t i = (size_t)(3 * (row * width + col)); // 3 = RGB
    pixels[i + 2] = pixels[i + 1] = pixels[i] = grey;
}

uint64_t get_time_in_GB_cycles_since(struct timeval *from)
{
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    if (!timercmp(&currTime, from, <))
    {
        struct timeval delta;
        timersub(&currTime, from, &delta);
        return delta.tv_sec * GB_CYCLES_PER_S + (delta.tv_usec * GB_CYCLES_PER_S) / 1000000;
    }
    return 0;
}
// ======================================================================
static void generate_image(guchar *pixels, int height, int width)
{
    uint64_t cycles = get_time_in_GB_cycles_since(&start);
    gameboy_run_until(&gameboy, cycles);

    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            uint8_t pixel = 0;
            if (image_get_pixel(&pixel, &gameboy.screen.display, (size_t)x / 3, (size_t)y / 3) != ERR_NONE)
                pixel = 0;
            set_grey(pixels, y, x, width, 255 - 85 * pixel);
        }
    }
}

// ======================================================================
#define do_key(X)                                  \
    do                                             \
    {                                              \
        if (!(psd->key_status & MY_KEY_##X##_BIT)) \
        {                                          \
            psd->key_status |= MY_KEY_##X##_BIT;   \
            puts(#X " key pressed");               \
        }                                          \
    } while (0)

static gboolean keypress_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t *const psd = data;
    if (psd == NULL)
        return FALSE;

    switch (keyval)
    {
    case GDK_KEY_Up:
        do_key(UP);
        joypad_key_pressed(&(gameboy.pad), UP_KEY);
        return TRUE;

    case GDK_KEY_Down:
        do_key(DOWN);
        joypad_key_pressed(&(gameboy.pad), DOWN_KEY);
        return TRUE;

    case GDK_KEY_Right:
        do_key(RIGHT);
        joypad_key_pressed(&(gameboy.pad), RIGHT_KEY);
        return TRUE;

    case GDK_KEY_Left:
        do_key(LEFT);
        joypad_key_pressed(&(gameboy.pad), LEFT_KEY);
        return TRUE;

    case 'A':
    case 'a':
        do_key(A);
        joypad_key_pressed(&(gameboy.pad), A_KEY);
        return TRUE;

    case 'B':
    case 'b':
        do_key(B);
        joypad_key_pressed(&(gameboy.pad), B_KEY);
        return TRUE;

    case GDK_KEY_Page_Up:
        do_key(SELECT);
        joypad_key_pressed(&(gameboy.pad), SELECT_KEY);
        return TRUE;

    case GDK_KEY_Page_Down:
        do_key(START);
        joypad_key_pressed(&(gameboy.pad), START_KEY);
        return TRUE;

    case GDK_KEY_space:
        if (psd->timeout_id > 0)
        {
            gettimeofday(&paused, NULL);
        }
        else
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            timersub(&now, &paused, &paused);
            timeradd(&paused, &start, &start);
            timerclear(&paused);
        }

        return ds_simple_key_handler(keyval, data);
    }
    return ds_simple_key_handler(keyval, data);
}
#undef do_key

// ======================================================================
#define do_key(X)                                                \
    do                                                           \
    {                                                            \
        if (psd->key_status & MY_KEY_##X##_BIT)                  \
        {                                                        \
            psd->key_status &= (unsigned char)~MY_KEY_##X##_BIT; \
            puts(#X " key released");                            \
        }                                                        \
    } while (0)

static gboolean keyrelease_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t *const psd = data;
    if (psd == NULL)
        return FALSE;

    switch (keyval)
    {
    case GDK_KEY_Up:
        do_key(UP);
        joypad_key_released(&(gameboy.pad), UP_KEY);
        return TRUE;

    case GDK_KEY_Down:
        do_key(DOWN);
        joypad_key_released(&(gameboy.pad), DOWN_KEY);
        return TRUE;

    case GDK_KEY_Right:
        do_key(RIGHT);
        joypad_key_released(&(gameboy.pad), RIGHT_KEY);
        return TRUE;

    case GDK_KEY_Left:
        do_key(LEFT);
        joypad_key_released(&(gameboy.pad), LEFT_KEY);
        return TRUE;

    case 'A':
    case 'a':
        do_key(A);
        joypad_key_released(&(gameboy.pad), A_KEY);
        return TRUE;

    case 'B':
    case 'b':
        do_key(B);
        joypad_key_released(&(gameboy.pad), B_KEY);
        return TRUE;

    case GDK_KEY_Page_Up:
        do_key(SELECT);
        joypad_key_released(&(gameboy.pad), SELECT_KEY);
        return TRUE;

    case GDK_KEY_Page_Down:
        do_key(START);
        joypad_key_released(&(gameboy.pad), START_KEY);
        return TRUE;
    }

    return FALSE;
}
#undef do_key

// ======================================================================
int main(int argc, char *argv[])
{
    zero_init_var(gameboy);
    if (argc < 2)
    {
        return 1;
    }
    M_EXIT_IF_ERR(gettimeofday(&start, NULL));
    M_EXIT_IF_ERR(timerclear(&paused));
    const char *const filename = argv[1];
    int err = gameboy_create(&gameboy, filename);
    if (err != ERR_NONE)
    {
        gameboy_free(&gameboy);
        return err;
    }

    sd_launch(&argc, &argv,
              sd_init("finalDemo", LCD_WIDTH * 3, LCD_HEIGHT * 3, 40,
                      generate_image, keypress_handler, keyrelease_handler));
    gameboy_free(&gameboy);

    return ERR_NONE;
}
