#include "../creese_2D.h"

#define RGFW_IMPLEMENTATION
#include "../external/RGFW.h"

static int win_width = 0;
static int win_height = 0;
#define SWR_FRAME_WIDTH win_width 
#define SWR_FRAME_HEIGHT win_height 
#define SWR_IMPLEMENTATION
#include "swr.h"

#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "../../nob.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

#define LA_IMPLEMENTATION
#include "../external/la.h"

/* modules */
#include "time_keep.c"

/* global variables local to this file */
static int win_x = 0;
static int win_y = 0;
static RGFW_window *window = NULL;
static RGFW_event event = {0};
static RGFW_surface *surface = NULL;
static uint8_t *frame_buff = NULL;

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

AABB_2D get_triangle_aabb(V2i v0, V2i v1, V2i v2)
{
    V4i aabb_2D;
    swr_triangle_aabb(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y, aabb_2D.c);
    return (AABB_2D) {
        .x_min = aabb_2D.x,
        .y_min = aabb_2D.y,
        .x_max = aabb_2D.z,
        .y_max = aabb_2D.w,
    };
}

void draw_aabb_2D(AABB_2D aabb, Color color)
{
    swr_draw_aabb_2D(frame_buff, aabb.x_min, aabb.y_min, aabb.x_max, aabb.y_max, color_to_uint32_t(color));
}

uint32_t color_to_uint32_t(Color color)
{
    uint32_t r = color.r;
    uint32_t g = color.g;
    uint32_t b = color.b;
    uint32_t a = color.a;
    return a << 24 | b << 16 | g << 8 | r;
}
