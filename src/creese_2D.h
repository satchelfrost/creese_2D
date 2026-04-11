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

typedef struct {
    int x;
    int y;
    int width;
    int height;
} Rectangle;

typedef struct {
    int width;
    int height;
    uint8_t *data;
} Image;

typedef struct {
    int x_min;
    int y_min;
    int x_max;
    int y_max;
} AABB_2D;

typedef struct {
   unsigned short x0, y0, x1, y1;
   float x_offset, y_offset, x_advance;
} Glyph;

#define CHAR_COUNT 96
#define FIRST_CHAR 32

typedef struct {
    float height;
    int bitmap_width;
    int bitmap_height;
    uint8_t *bitmap;
    Glyph glyphs[CHAR_COUNT]; // ASCII 32..126 is 95 glyphs
} Font;

typedef struct {
    int x, y;
} Mouse;

/* general */
uint8_t *get_frame_buffer();
void init_window(int width, int height, char *title);
void close_window(void);
bool window_should_close(void);
void begin_drawing(Color bg_color);
void end_drawing(void);
void clear_background(Color bg_color); // also called by begin_drawing

/* simple shapes */
void draw_pixel(int x, int y, Color color);
void draw_circle(int x, int y, int radius, Color color);
void draw_triangle(V2i v0, V2i v1, V2i v2, Color color);
void draw_triangle_wireframe(V2i v0, V2i v1, V2i v2, Color color);
void draw_rectangle(Rectangle rectangle, Color color);
void draw_rectangle_lines(Rectangle rectangle, Color color);
void draw_line(int x0, int y0, int x1, int y1, Color color);

/* Image */
Image load_image(const char *image_path);
void unload_image(Image image);
void draw_image(Image image, int x, int y);
void draw_image_scaled(Image image, int x, int y, int scale_x, int scale_y);
void draw_image_rect(Image image, Rectangle r, int x, int y);
void draw_image_rect_scaled(Image image, Rectangle r, int x, int y, int scale_x, int scale_y);

/* Text */
Font load_font(const char *file_path, int font_height);
void unload_font(Font font);
void draw_text_at_base(Font font, const char *text, size_t text_len, int x, int y, Color color);
void draw_text_at_base_scaled(Font font, const char *text, size_t text_len, int x, int y, Color color, int scale_x, int scale_y);

/* misc */
Rectangle get_bounding_rectangle_triangle(V2i v0, V2i v1, V2i v2);
uint32_t color_to_uint32_t(Color color);

Mouse get_mouse_position();

/* time_keep.c */
void begin_timer();
void end_timer();
double get_frame_time();
double get_time();
int get_fps();
void log_fps();
void wait_time(double seconds);

#endif // CREESE_2D_H_
