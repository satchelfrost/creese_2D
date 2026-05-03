#ifndef CREESE_2D_H_
#define CREESE_2D_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LADEF
#include "./external/la.h"

/* colors stolen from raylib - https://github.com/raysan5/raylib */
#define LIGHTGRAY  (Color){ 200, 200, 200, 255 }
#define GRAY       (Color){ 130, 130, 130, 255 }
#define DARKGRAY   (Color){ 80, 80, 80, 255 }
#define YELLOW     (Color){ 253, 249, 0, 255 }
#define GOLD       (Color){ 255, 203, 0, 255 }
#define ORANGE     (Color){ 255, 161, 0, 255 }
#define PINK       (Color){ 255, 109, 194, 255 }
#define RED        (Color){ 230, 41, 55, 255 }
#define MAROON     (Color){ 190, 33, 55, 255 }
#define GREEN      (Color){ 0, 228, 48, 255 }
#define LIME       (Color){ 0, 158, 47, 255 }
#define DARKGREEN  (Color){ 0, 117, 44, 255 }
#define SKYBLUE    (Color){ 102, 191, 255, 255 }
#define BLUE       (Color){ 0, 121, 241, 255 }
#define DARKBLUE   (Color){ 0, 82, 172, 255 }
#define PURPLE     (Color){ 200, 122, 255, 255 }
#define VIOLET     (Color){ 135, 60, 190, 255 }
#define DARKPURPLE (Color){ 112, 31, 126, 255 }
#define BEIGE      (Color){ 211, 176, 131, 255 }
#define BROWN      (Color){ 127, 106, 79, 255 }
#define DARKBROWN  (Color){ 76, 63, 47, 255 }
#define WHITE      (Color){ 255, 255, 255, 255 }
#define BLACK      (Color){ 0, 0, 0, 255 }
#define BLANK      (Color){ 0, 0, 0, 0 }
#define MAGENTA    (Color){ 255, 0, 255, 255 }

/* input mappings stolen from raylib - https://github.com/raysan5/raylib */
typedef enum {
    KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    // Alphanumeric keys
    KEY_APOSTROPHE      = 39,       // Key: '
    KEY_COMMA           = 44,       // Key: ,
    KEY_MINUS           = 45,       // Key: -
    KEY_PERIOD          = 46,       // Key: .
    KEY_SLASH           = 47,       // Key: /
    KEY_ZERO            = 48,       // Key: 0
    KEY_ONE             = 49,       // Key: 1
    KEY_TWO             = 50,       // Key: 2
    KEY_THREE           = 51,       // Key: 3
    KEY_FOUR            = 52,       // Key: 4
    KEY_FIVE            = 53,       // Key: 5
    KEY_SIX             = 54,       // Key: 6
    KEY_SEVEN           = 55,       // Key: 7
    KEY_EIGHT           = 56,       // Key: 8
    KEY_NINE            = 57,       // Key: 9
    KEY_SEMICOLON       = 59,       // Key: ;
    KEY_EQUAL           = 61,       // Key: =
    KEY_A               = 65,       // Key: A | a
    KEY_B               = 66,       // Key: B | b
    KEY_C               = 67,       // Key: C | c
    KEY_D               = 68,       // Key: D | d
    KEY_E               = 69,       // Key: E | e
    KEY_F               = 70,       // Key: F | f
    KEY_G               = 71,       // Key: G | g
    KEY_H               = 72,       // Key: H | h
    KEY_I               = 73,       // Key: I | i
    KEY_J               = 74,       // Key: J | j
    KEY_K               = 75,       // Key: K | k
    KEY_L               = 76,       // Key: L | l
    KEY_M               = 77,       // Key: M | m
    KEY_N               = 78,       // Key: N | n
    KEY_O               = 79,       // Key: O | o
    KEY_P               = 80,       // Key: P | p
    KEY_Q               = 81,       // Key: Q | q
    KEY_R               = 82,       // Key: R | r
    KEY_S               = 83,       // Key: S | s
    KEY_T               = 84,       // Key: T | t
    KEY_U               = 85,       // Key: U | u
    KEY_V               = 86,       // Key: V | v
    KEY_W               = 87,       // Key: W | w
    KEY_X               = 88,       // Key: X | x
    KEY_Y               = 89,       // Key: Y | y
    KEY_Z               = 90,       // Key: Z | z
    KEY_LEFT_BRACKET    = 91,       // Key: [
    KEY_BACKSLASH       = 92,       // Key: '\'
    KEY_RIGHT_BRACKET   = 93,       // Key: ]
    KEY_GRAVE           = 96,       // Key: `
    // Function keys
    KEY_SPACE           = 32,       // Key: Space
    KEY_ESCAPE          = 256,      // Key: Esc
    KEY_ENTER           = 257,      // Key: Enter
    KEY_TAB             = 258,      // Key: Tab
    KEY_BACKSPACE       = 259,      // Key: Backspace
    KEY_INSERT          = 260,      // Key: Ins
    KEY_DELETE          = 261,      // Key: Del
    KEY_RIGHT           = 262,      // Key: Cursor right
    KEY_LEFT            = 263,      // Key: Cursor left
    KEY_DOWN            = 264,      // Key: Cursor down
    KEY_UP              = 265,      // Key: Cursor up
    KEY_PAGE_UP         = 266,      // Key: Page up
    KEY_PAGE_DOWN       = 267,      // Key: Page down
    KEY_HOME            = 268,      // Key: Home
    KEY_END             = 269,      // Key: End
    KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    KEY_NUM_LOCK        = 282,      // Key: Num lock
    KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    KEY_PAUSE           = 284,      // Key: Pause
    KEY_F1              = 290,      // Key: F1
    KEY_F2              = 291,      // Key: F2
    KEY_F3              = 292,      // Key: F3
    KEY_F4              = 293,      // Key: F4
    KEY_F5              = 294,      // Key: F5
    KEY_F6              = 295,      // Key: F6
    KEY_F7              = 296,      // Key: F7
    KEY_F8              = 297,      // Key: F8
    KEY_F9              = 298,      // Key: F9
    KEY_F10             = 299,      // Key: F10
    KEY_F11             = 300,      // Key: F11
    KEY_F12             = 301,      // Key: F12
    KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    KEY_LEFT_CONTROL    = 341,      // Key: Control left
    KEY_LEFT_ALT        = 342,      // Key: Alt left
    KEY_LEFT_SUPER      = 343,      // Key: Super left
    KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    KEY_RIGHT_ALT       = 346,      // Key: Alt right
    KEY_RIGHT_SUPER     = 347,      // Key: Super right
    KEY_KB_MENU         = 348,      // Key: KB menu
    // Keypad keys
    KEY_KP_0            = 320,      // Key: Keypad 0
    KEY_KP_1            = 321,      // Key: Keypad 1
    KEY_KP_2            = 322,      // Key: Keypad 2
    KEY_KP_3            = 323,      // Key: Keypad 3
    KEY_KP_4            = 324,      // Key: Keypad 4
    KEY_KP_5            = 325,      // Key: Keypad 5
    KEY_KP_6            = 326,      // Key: Keypad 6
    KEY_KP_7            = 327,      // Key: Keypad 7
    KEY_KP_8            = 328,      // Key: Keypad 8
    KEY_KP_9            = 329,      // Key: Keypad 9
    KEY_KP_DECIMAL      = 330,      // Key: Keypad .
    KEY_KP_DIVIDE       = 331,      // Key: Keypad /
    KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
    KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
    KEY_KP_ADD          = 334,      // Key: Keypad +
    KEY_KP_ENTER        = 335,      // Key: Keypad Enter
    KEY_KP_EQUAL        = 336,      // Key: Keypad =
    // Android key buttons
    KEY_BACK            = 4,        // Key: Android back button
    KEY_MENU            = 5,        // Key: Android menu button
    KEY_VOLUME_UP       = 24,       // Key: Android volume up button
    KEY_VOLUME_DOWN     = 25        // Key: Android volume down button
} Keyboard_Key;

typedef enum {
    MOUSE_BUTTON_LEFT    = 0,
    MOUSE_BUTTON_MIDDLE  = 1,
    MOUSE_BUTTON_RIGHT   = 2,
} Mouse_Button;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

/* general */
uint8_t *get_frame_buffer();
void init_window(int width, int height, char *title);
void close_window(void);
bool window_should_close(void);
void *get_window_ptr();
void begin_drawing(Color bg_color);
void end_drawing(void);
void clear_background(Color bg_color); // also called by begin_drawing

/* simple shapes */
typedef struct {
    int x;
    int y;
    int width;
    int height;
} Rectangle;

void draw_pixel(int x, int y, Color color);
void draw_circle(int x, int y, int radius, Color color);
void draw_triangle(V2i v0, V2i v1, V2i v2, Color color);
void draw_triangle_wireframe(V2i v0, V2i v1, V2i v2, Color color);
void draw_rectangle(Rectangle rectangle, Color color);
void draw_rectangle_lines(Rectangle rectangle, Color color);
void draw_line(int x0, int y0, int x1, int y1, Color color);

/* Image */
typedef struct {
    int width;
    int height;
    uint8_t *data;
} Image;

Image load_image(const char *image_path);
void unload_image(Image image);
void draw_image(Image image, int x, int y);
void draw_image_flip_x(Image image, int x, int y);
void draw_image_flip_y(Image image, int x, int y);
void draw_image_scaled(Image image, int x, int y, int scale_x, int scale_y);
void draw_image_scaled_tint(Image image, int x, int y, int scale_x, int scale_y, Color tint);
void draw_image_scaled_down(Image image, int x, int y, int scale_x, int scale_y);
void draw_image_scaled_down_tint(Image image, int x, int y, int scale_x, int scale_y, Color tint);
void draw_image_rect(Image image, Rectangle r, int x, int y);
void draw_image_rect_scaled(Image image, Rectangle r, int x, int y, int scale_x, int scale_y);
void draw_image_rect_scaled_flip_x(Image image, Rectangle r, int x, int y, int scale_x, int scale_y);

/* Text */
#define CHAR_COUNT 96
#define FIRST_CHAR 32

typedef struct {
   unsigned short x0, y0, x1, y1;
   float x_offset, y_offset, x_advance;
} Glyph;

typedef struct {
    float height;
    int bitmap_width;
    int bitmap_height;
    uint8_t *bitmap;
    Glyph glyphs[CHAR_COUNT]; // ASCII 32..126 is 95 glyphs
} Font;

Font load_font(const char *file_path, int font_height);
void unload_font(Font font);
void draw_text_at_base(Font font, const char *text, size_t text_len, int x, int y, Color color);
void draw_text_at_base_scaled(Font font, const char *text, size_t text_len, int x, int y, Color color, int scale_x, int scale_y);

/* sprite.c */
typedef struct {
    float scale;

    struct {
        bool playing;
        float time;
        float timeout;
        uint32_t frame;
        uint32_t type;
        bool horizontal; // if true animation frames are left/right, as opposed to top/bottom
    } animation;

    Image image;
    struct {
        V2f size;
        V2f shift_amt;
        V2f offset;
        Rectangle rect;
    } sub_image;
} Sprite;

Rectangle get_anim_sub_rect(Sprite sprite);
void update_animation(Sprite *sprite, uint32_t total_anim_frames);
Sprite load_sprite_from_image(Image image, uint32_t horizontal_sprite_count, uint32_t vertical_sprite_count, float scale);
void draw_sprite(Sprite sprite, int x, int y);
void draw_sprite_flip_x(Sprite sprite, int x, int y);
void draw_sprite_centered(Sprite sprite, int x, int y);
void draw_sprite_centered_debug(Sprite sprite, int x, int y);

bool is_key_pressed(Keyboard_Key key);
bool is_key_released(Keyboard_Key key);
bool is_key_down(Keyboard_Key key);
bool mouse_inside_window();
V2i get_mouse_position();
bool is_mouse_button_pressed(Mouse_Button button);
bool is_mouse_button_released(Mouse_Button button);
bool is_mouse_button_down(Mouse_Button button);
V2f get_mouse_wheel_move();
V2f get_mouse_vector();

/* time_keep.c */
void begin_timer();
void end_timer();
double get_frame_time();
double get_time();
int get_fps();
int get_avg_fps();
void log_fps();
void wait_time(double seconds);

/* audio.c */
typedef struct Audio_Buffer Audio_Buffer;

typedef struct {
    Audio_Buffer *buffer;
    uint32_t sample_rate;
    uint32_t sample_size;
    uint32_t channels;
} Audio_Stream;

typedef struct {
    Audio_Stream stream;
    uint32_t frame_count;
} Sound;

typedef struct {
    Audio_Stream stream;
    uint32_t frame_count;
    bool looping;
    void *data;
} Music;

void init_audio_device(void);
void close_audio_device(void);

Sound load_sound(const char *file_path); // use sound if < 10 seconds
void unload_sound(Sound sound);
void play_sound(Sound sound);
void stop_sound(Sound sound);
void pause_sound(Sound sound);
void resume_sound(Sound sound);
bool is_sound_playing(Sound sound);
void set_sound_volume(Sound sound, float volume); // range 0 - 1.0
void set_sound_pitch(Sound sound, float pitch);   // range 0 - 1.0
void set_sound_pan(Sound sound, float pan);       // -1 = left, 0 = center, +1 = right

Music load_music_stream(const char *file_path); // use music if > 10 seconds
void unload_music_stream(Music music);
void play_music_stream(Music music);
void stop_music_stream(Music music);
void update_music_stream(Music music);
void pause_music_stream(Music music);
void resume_music_stream(Music music);

/* misc */
Rectangle get_bounding_rectangle_triangle(V2i v0, V2i v1, V2i v2);
uint32_t color_to_uint32_t(Color color);
bool rectangle_collision(Rectangle r0, Rectangle r1);
bool rectangle_circle_collision(Rectangle r, int cx, int cy, int radius);
bool circle_circle_collision(int c0x, int c0y, int r0, int c1x, int c1y, int r1);

#endif // CREESE_2D_H_
