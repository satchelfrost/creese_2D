#include "../creese_2D.h"

static int win_width = 0;
static int win_height = 0;
#define SWR_FRAME_WIDTH win_width 
#define SWR_FRAME_HEIGHT win_height 
#define SWR_IMPLEMENTATION
#include "swr.h"

#define NOB_STRIP_PREFIX
#include "../../nob.h"
#include "../external/stb_image.h"
#include "../external/stb_truetype.h"

/* modules */
#include "time_keep.c"

/* global variables local to this file */
static int win_x = 0;
static int win_y = 0;
static RGFW_window *window = NULL;
static RGFW_event event = {0};
static RGFW_surface *surface = NULL;
static uint8_t *frame_buff = NULL;

#define DEFAULT_BITMAP_WIDTH 180
#define DEFAULT_BITMAP_HEIGHT 170

uint8_t *get_frame_buffer()
{
    return frame_buff;
}

void init_window(int width, int height, char *title)
{
    win_width = width;
    win_height = height;
    window = RGFW_createWindow(title, win_x, win_y, width, height, (u64)0);
    frame_buff = malloc(SWR_FRAME_WIDTH*SWR_FRAME_HEIGHT*4);
    surface = RGFW_createSurface(frame_buff, SWR_FRAME_WIDTH, SWR_FRAME_HEIGHT, RGFW_formatRGBA8);
	RGFW_window_setExitKey(window, RGFW_escape);
}

void close_window()
{
    RGFW_surface_free(surface);
	RGFW_window_close(window);
    free(frame_buff);
}

bool window_should_close()
{
    bool result = RGFW_window_shouldClose(window);
    while (RGFW_window_checkEvent(window, &event)) {
        if (event.type == RGFW_quit) result = true;
    }
    return result;
}

void begin_drawing(Color bg_color)
{
    begin_timer();
    swr_clear_background(frame_buff, color_to_uint32_t(bg_color));
}

void end_drawing()
{
    RGFW_window_blitSurface(window, surface);
    end_timer();
}

void clear_background(Color bg_color)
{
    swr_clear_background(frame_buff, color_to_uint32_t(bg_color));
}

void draw_pixel(int x, int y, Color color)
{
    swr_put_pixel(frame_buff, x, y, color_to_uint32_t(color));
}

void draw_circle(int x, int y, int radius, Color color)
{
    swr_draw_circle(frame_buff, x, y, radius, color_to_uint32_t(color));
}

void draw_triangle(V2i v0, V2i v1, V2i v2, Color color)
{
    swr_draw_triangle(frame_buff, v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, color_to_uint32_t(color));
}

void draw_triangle_wireframe(V2i v0, V2i v1, V2i v2, Color color)
{
    swr_draw_triangle_wireframe(frame_buff, v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, color_to_uint32_t(color));
}

void draw_rectangle(Rectangle r, Color color)
{
    swr_draw_rectangle(frame_buff, r.x, r.y, r.width, r.height, color_to_uint32_t(color));
}

void draw_rectangle_lines(Rectangle rectangle, Color color)
{
    swr_draw_aabb_2D(frame_buff, rectangle.x, rectangle.y, rectangle.width, rectangle.height, color_to_uint32_t(color));
}

void draw_line(int x0, int y0, int x1, int y1, Color color)
{
    swr_draw_line(frame_buff, x0, y0, x1, y1, color_to_uint32_t(color));
}

Image load_image(const char *image_path)
{
    Image img = {0};
    int num_components;
    img.data = stbi_load(image_path, &img.width, &img.height, &num_components, 0);
    if (!img.data) {
        printf("ERROR: failed to load image\n");
        return img;
    }
    return img;
}

void unload_image(Image image)
{
    free(image.data);
}

void draw_image(Image image, int x, int y)
{
    swr_draw_image(frame_buff, image.data, x, y, image.width, image.height);
}

void draw_image_scaled(Image image, int x, int y, int scale_x, int scale_y)
{
    swr_draw_image_scaled(frame_buff, image.data, x, y, image.width, image.height, scale_x, scale_y);
}

void draw_image_rect(Image image, Rectangle r, int x, int y)
{
    swr_draw_image_rect(frame_buff, image.data, x, y, image.width, image.height,
                        r.x, r.y, r.width, r.height);
}

void draw_image_rect_scaled(Image image, Rectangle r, int x, int y, int scale_x, int scale_y)
{
    swr_draw_image_rect_scaled(frame_buff, image.data, x, y, image.width, image.height,
                               r.x, r.y, r.width, r.height, scale_x, scale_y);
}

Font load_font(const char *file_path, int font_height)
{
    bool result = true;

    Font font = {
        .bitmap_width = DEFAULT_BITMAP_WIDTH,
        .bitmap_height = DEFAULT_BITMAP_HEIGHT,
        .bitmap = malloc(DEFAULT_BITMAP_WIDTH*DEFAULT_BITMAP_HEIGHT),
        .height = font_height,
    };

    String_Builder sb = {0};
    if (!read_entire_file(file_path, &sb)) return_defer(false);
    stbtt_BakeFontBitmap((unsigned char *)sb.items, 0, font.height, font.bitmap,
                         font.bitmap_width, font.bitmap_height, FIRST_CHAR, CHAR_COUNT, (stbtt_bakedchar*)font.glyphs);

defer:
    sb_free(sb);
    if (!result) {
        free(font.bitmap);
    }
    return font;
}

void unload_font(Font font)
{
    free(font.bitmap);
}

void draw_text_at_base(Font font, const char *text, size_t text_len, int x, int y, Color color)
{
    int x_adv = 0;
    for (size_t c = 0; c < text_len; c++) {
        uint8_t ch = text[c];
        if (!(FIRST_CHAR <= ch && ch < (CHAR_COUNT + FIRST_CHAR))) continue;
        Glyph glyph = font.glyphs[ch-FIRST_CHAR];
        int i, j;   // bitmap index
        int xp, yp; // pen position
        for (i = glyph.y0, yp = y; i <= glyph.y1; i++, yp++) {
            for (j = glyph.x0, xp = x; j <= glyph.x1; j++, xp++) {
                color.a = font.bitmap[i*DEFAULT_BITMAP_WIDTH + j];
                int coord_x = xp + glyph.x_offset + x_adv;
                int coord_y = yp + glyph.y_offset;
                if (color.a) draw_pixel(coord_x, coord_y, color);
            }
        }
        x_adv += glyph.x_advance;
    }
}

Rectangle get_bounding_rectangle_triangle(V2i v0, V2i v1, V2i v2)
{
    V4i aabb_2D;
    swr_triangle_aabb(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, aabb_2D.c);
    return (Rectangle) {
        .x      = aabb_2D.x,
        .y      = aabb_2D.y,
        .width  = aabb_2D.z,
        .height = aabb_2D.w,
    };
}

uint32_t color_to_uint32_t(Color color)
{
    uint32_t r = color.r;
    uint32_t g = color.g;
    uint32_t b = color.b;
    uint32_t a = color.a;
    return a << 24 | b << 16 | g << 8 | r;
}
