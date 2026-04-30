#ifndef CREESE_2D_H_
#define CREESE_2D_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LADEF
#include "./external/la.h"

#define RGFWDEF extern
#include "./external/RGFW.h"

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
void draw_sprite_centered(Sprite sprite, int x, int y);
void draw_sprite_centered_debug(Sprite sprite, int x, int y);

typedef struct {
    int x, y;
} Mouse;

Mouse get_mouse_position();
bool mouse_inside_window();

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
