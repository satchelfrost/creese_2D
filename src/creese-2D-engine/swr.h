///////////////////////////////////////////
///////// swr - software renderer /////////
//////////////////////////////////////////

#ifndef SWR_H_
#define SWR_H_

#ifndef SWR_FRAME_WIDTH
#define SWR_FRAME_WIDTH 500
#endif

#ifndef SWR_FRAME_HEIGHT
#define SWR_FRAME_HEIGHT 500
#endif

#define swr_absi(x) ((x) > 0 ? (x) : -(x))
#define swr_swapi(x, y) do { int tmp = x; x = y; y = tmp; } while (0)
#define swr_min(a, b) (a) < (b) ? (a) : (b)
#define swr_max(a, b) (a) > (b) ? (a) : (b)

////////////////////////////
///////// base API /////////
////////////////////////////

void swr_clear_background(uint8_t *buff, uint32_t color);
void swr_put_pixel(uint8_t *buff, int x, int y, uint32_t color);
void swr_draw_circle(uint8_t *buff, int x0, int y0, int radius, uint32_t color);
void swr_draw_rectangle(uint8_t *buff, int x0, int y0, int w, int h, uint32_t color);
void swr_draw_image(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h);
void swr_draw_image_flip_x(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h);
void swr_draw_image_flip_y(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h);
void swr_draw_image_rect(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int rect_x, int rect_y, int rect_w, int rect_h);
void swr_draw_image_rect_flip_x(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int rect_x, int rect_y, int rect_w, int rect_h);
void swr_draw_image_rect_scaled(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int rect_x, int rect_y, int rect_w, int rect_h, int scale_x, int scale_y);
void swr_draw_image_rect_scaled_flip_x(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int rect_x, int rect_y, int rect_w, int rect_h, int scale_x, int scale_y);
void swr_draw_image_scaled(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int scale_x, int scale_y);
void swr_draw_image_scaled_tint(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int scale_x, int scale_y, uint32_t tint);
void swr_draw_image_scaled_down(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int scale_x, int scale_y);
void swr_draw_image_scaled_down_tint(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int scale_x, int scale_y, uint32_t tint);
void swr_draw_line(uint8_t *buff, int x0, int y0, int x1, int y1, uint32_t color);
void swr_draw_triangle_wireframe(uint8_t *buff, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void swr_draw_triangle(uint8_t *buff, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
int swr_count_pixel_color(uint8_t * buff, uint32_t color);

/* aabb_2D_out - x_min, y_min, x_max, and y_max respectively */
void swr_triangle_aabb(int x0, int y0, int x1, int y1, int x2, int y2, int aabb_2D_out[4]);
void swr_draw_aabb_2D(uint8_t *buff, int x_min, int y_min, int x_max, int y_max, uint32_t color);

#endif // SWR_H_

//////////////////////////////////
///////// implementation /////////
//////////////////////////////////

#ifdef SWR_IMPLEMENTATION

void swr_clear_background(uint8_t *buff, uint32_t color)
{
    for (int i = 0; i < SWR_FRAME_WIDTH*SWR_FRAME_HEIGHT; i++) ((uint32_t *)buff)[i] = color;
}

uint32_t swr_tint(uint32_t color_a, uint32_t color_b)
{
    uint32_t color_a_r = (color_a >> 0 & 0xff);
    uint32_t color_a_g = (color_a >> 8 & 0xff);
    uint32_t color_a_b = (color_a >>16 & 0xff);
    uint32_t color_a_a = (color_a >>24 & 0xff);
    uint32_t color_b_r = (color_b >> 0 & 0xff);
    uint32_t color_b_g = (color_b >> 8 & 0xff);
    uint32_t color_b_b = (color_b >>16 & 0xff);
    uint32_t res_r = (color_a_r + color_b_r)/2;
    uint32_t res_g = (color_a_g + color_b_g)/2;
    uint32_t res_b = (color_a_b + color_b_b)/2;
    uint32_t res_a = color_a_a;

    return res_a<<24 | res_b<<16 | res_g<<8 | res_r;
}

uint32_t swr_alpha_blend(uint32_t top, uint32_t bottom)
{
    uint32_t top_a = (top>>24 & 0xff);
    uint32_t top_r = (top>> 0 & 0xff) * top_a;
    uint32_t top_g = (top>> 8 & 0xff) * top_a;
    uint32_t top_b = (top>>16 & 0xff) * top_a;
    uint32_t bottom_r = (bottom>> 0 & 0xff) * (255 - top_a);
    uint32_t bottom_g = (bottom>> 8 & 0xff) * (255 - top_a);
    uint32_t bottom_b = (bottom>>16 & 0xff) * (255 - top_a);
    uint32_t bottom_a = (bottom>>24 & 0xff) * (255 - top_a);
    uint32_t res_r = (top_r + bottom_r)/255;
    uint32_t res_g = (top_g + bottom_g)/255;
    uint32_t res_b = (top_b + bottom_b)/255;
    uint32_t res_a = (top_a + bottom_a)/255;

    return res_a<<24 | res_b<<16 | res_g<<8 | res_r;
}

void swr_draw_circle(uint8_t *buff, int x, int y, int radius, uint32_t color)
{
    /* make the start position the top left of a square around the circle */
    int y0 = y - radius;
    int x0 = x - radius;

    for (int i = 0; i < radius*2; i++) {
        int yp = y0 + i;
        int dy = y - yp;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue;
        for (int j = 0; j < radius*2; j++) {
            int xp = x0 + j;
            int dx = x - xp;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue;
            if ((dx*dx + dy*dy) < radius*radius) {
                swr_put_pixel(buff, xp, yp, color);
            }
        }
    }
}

void swr_draw_rectangle(uint8_t *buff, int x0, int y0, int w, int h, uint32_t color)
{
    for (int y = 0; y < h; y++) {
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue;
        for (int x = 0; x < w; x++) {
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue;
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h)
{
    for (int y = 0; y < h; y++) {
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue;
        for (int x = 0; x < w; x++) {
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue;
            uint32_t color = ((uint32_t *)image)[y*w + x];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_flip_x(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h)
{
    for (int y = 0; y < h; y++) {
        int yp = y0 + y;
        int yi = y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue;
        for (int x = 0; x < w; x++) {
            int xp = x0 + x;
            int xi = w - 1 - x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue;
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_flip_y(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h)
{
    for (int y = 0; y < h; y++) {
        int yp = y0 + y;
        int yi = h - 1 - y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue;
        for (int x = 0; x < w; x++) {
            int xp = x0 + x;
            int xi = x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue;
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_rect(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int rect_x, int rect_y, int rect_w, int rect_h)
{
    for (int y = 0; y < rect_h; y++) {
        int yi = rect_y + y;
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue; // frame bounds check
        if (!(0 <= yi && yi < h))                continue; // image bounds check
        for (int x = 0; x < rect_w; x++) {
            int xi = rect_x + x;
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue; // frame bounds check
            if (!(0 <= xi && xi < w))               continue; // image bounds check
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_rect_flip_x(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int rect_x, int rect_y, int rect_w, int rect_h)
{
    for (int y = 0; y < rect_h; y++) {
        int yi = rect_y + y;
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue; // frame bounds check
        if (!(0 <= yi && yi < h))                continue; // image bounds check
        for (int x = 0; x < rect_w; x++) {
            int xi = rect_x + rect_w - 1 - x;
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue; // frame bounds check
            if (!(0 <= xi && xi < w))               continue; // image bounds check
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_rect_scaled(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int rect_x, int rect_y, int rect_w, int rect_h, int scale_x, int scale_y)
{
    for (int y = 0; y < rect_h*scale_y; y++) {
        int yi = rect_y + y/scale_y;
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue; // frame bounds check
        if (!(0 <= yi && yi < h))                continue; // image bounds check
        for (int x = 0; x < rect_w*scale_x; x++) {
            int xi = rect_x + x/scale_x;
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue; // frame bounds check
            if (!(0 <= xi && xi < w))               continue; // image bounds check
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_rect_scaled_flip_x(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int rect_x, int rect_y, int rect_w, int rect_h, int scale_x, int scale_y)
{
    for (int y = 0; y < rect_h*scale_y; y++) {
        int yi = rect_y + y/scale_y;
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue; // frame bounds check
        if (!(0 <= yi && yi < h))                continue; // image bounds check
        for (int x = 0; x < rect_w*scale_x; x++) {
            int xi = rect_x + rect_w - 1 - x/scale_x;
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue; // frame bounds check
            if (!(0 <= xi && xi < w))               continue; // image bounds check
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_scaled(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int scale_x, int scale_y)
{
    for (int y = 0; y < h*scale_y; y++) {
        int yi = y/scale_y;
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue; // frame bounds check
        if (!(0 <= yi && yi < h))                continue; // image bounds check
        for (int x = 0; x < w*scale_x; x++) {
            int xi = x/scale_x;
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue; // frame bounds check
            if (!(0 <= xi && xi < w))               continue; // image bounds check
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_scaled_tint(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int scale_x, int scale_y, uint32_t tint)
{
    for (int y = 0; y < h*scale_y; y++) {
        int yi = y/scale_y;
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue; // frame bounds check
        if (!(0 <= yi && yi < h))                continue; // image bounds check
        for (int x = 0; x < w*scale_x; x++) {
            int xi = x/scale_x;
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue; // frame bounds check
            if (!(0 <= xi && xi < w))               continue; // image bounds check
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, swr_tint(color, tint));
        }
    }
}

void swr_draw_image_scaled_down(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int scale_x, int scale_y)
{
    for (int y = 0; y < h/scale_y; y++) {
        int yi = y*scale_y;
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue; // frame bounds check
        if (!(0 <= yi && yi < h))                continue; // image bounds check
        for (int x = 0; x < w/scale_x; x++) {
            int xi = x*scale_x;
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue; // frame bounds check
            if (!(0 <= xi && xi < w))               continue; // image bounds check
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, color);
        }
    }
}

void swr_draw_image_scaled_down_tint(uint8_t *buff, uint8_t *image, int x0, int y0, int w, int h, int scale_x, int scale_y, uint32_t tint)
{
    for (int y = 0; y < h/scale_y; y++) {
        int yi = y*scale_y;
        int yp = y0 + y;
        if (!(0 <= yp && yp < SWR_FRAME_HEIGHT)) continue; // frame bounds check
        if (!(0 <= yi && yi < h))                continue; // image bounds check
        for (int x = 0; x < w/scale_x; x++) {
            int xi = x*scale_x;
            int xp = x0 + x;
            if (!(0 <= xp && xp < SWR_FRAME_WIDTH)) continue; // frame bounds check
            if (!(0 <= xi && xi < w))               continue; // image bounds check
            uint32_t color = ((uint32_t *)image)[yi*w + xi];
            swr_put_pixel(buff, xp, yp, swr_tint(color, tint));
        }
    }
}

void swr_put_pixel(uint8_t *buff, int x, int y, uint32_t color)
{
    if (!((0 <= x && x < SWR_FRAME_WIDTH) && (0 <= y && y < SWR_FRAME_HEIGHT))) return;
    uint32_t src = ((uint32_t *)buff)[y*SWR_FRAME_WIDTH + x];
    ((uint32_t *)buff)[y*SWR_FRAME_WIDTH + x] = swr_alpha_blend(color, src);
}

void swr_draw_line(uint8_t *buff, int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    if (dy == 0 && dx == 0) {
        swr_put_pixel(buff, x0, y0, color);
        return;
    }

    if (swr_absi(dx) > swr_absi(dy)) {
        if (x0 > x1) {
            swr_swapi(x0, x1);
            swr_swapi(y0, y1);
        }
        for (int x = x0; x <= x1; x++) {
            int y = (x - x0)*dy/dx + y0;
            swr_put_pixel(buff, x, y, color);
        }
    } else {
        if (y0 > y1) {
            swr_swapi(y0, y1);
            swr_swapi(x0, x1);
        }
        for (int y = y0; y <= y1; y++) {
            int x = (y - y0)*dx/dy + x0;
            swr_put_pixel(buff, x, y, color);
        }
    }
}

void swr_draw_triangle_wireframe(uint8_t *buff, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    swr_draw_line(buff, x0, y0, x1, y1, color);
    swr_draw_line(buff, x1, y1, x2, y2, color);
    swr_draw_line(buff, x2, y2, x0, y0, color);
}

void swr_draw_triangle(uint8_t *buff, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    if (y0 > y1) {
        swr_swapi(y0, y1);
        swr_swapi(x0, x1);
    }
    if (y0 > y2) {
        swr_swapi(y0, y2);
        swr_swapi(x0, x2);
    }
    if (y1 > y2) {
        swr_swapi(y1, y2);
        swr_swapi(x1, x2);
    }

    int dx0 = x1 - x0;
    int dy0 = y1 - y0;
    int dx1 = x1 - x2;
    int dy1 = y1 - y2;
    int dx2 = x2 - x0;
    int dy2 = y2 - y0;

    /*
     *                    (x0, y0)
     *                     .
     *                     |\
     *                     | \  <-- short side 1 (y < y1)
     *                     |  \
     *                     |   \
     *   longest side -->  |    . (x1, y1)
     *                     |   /
     *                     |  /  <-- short side 2 (y >= y1)
     *                     | /
     *                     |/
     *                     . (x2, y2)
     *
     * */

    for (int y = y0; y <= y2; y++) {
        int long_side_x  = (dy2) ? (y - y0)*dx2/dy2 + x0: x0;
        int short_side_x = 0;
        if (y < y1) short_side_x = (dy0) ? (y - y0)*dx0/dy0 + x0 : x0;
        else        short_side_x = (dy1) ? (y - y1)*dx1/dy1 + x1 : x1;
        int left_x  = swr_min(long_side_x, short_side_x);
        int right_x = swr_max(long_side_x, short_side_x);

        for (int x = left_x; x <= right_x; x++)
            swr_put_pixel(buff, x, y, color);
    }
}

int swr_count_pixel_color(uint8_t *buff, uint32_t color)
{
    int count = 0;
    for (int i = 0; i < SWR_FRAME_WIDTH*SWR_FRAME_HEIGHT; i++) count += color == ((uint32_t *)buff)[i];
    return count;
}

void swr_triangle_aabb(int x0, int y0, int x1, int y1, int x2, int y2, int aabb_2D[4])
{
    if (!aabb_2D) return;
        
    int x_min = swr_min(swr_min(x0, x1), x2);
    int y_min = swr_min(swr_min(y0, y1), y2);
    int x_max = swr_max(swr_max(x0, x1), x2);
    int y_max = swr_max(swr_max(y0, y1), y2);
    aabb_2D[0] = x_min;
    aabb_2D[1] = y_min;
    aabb_2D[2] = x_max;
    aabb_2D[3] = y_max;
}

void swr_draw_aabb_2D(uint8_t *buff, int x_min, int y_min, int x_max, int y_max, uint32_t color)
{
    swr_draw_line(buff, x_min, y_min, x_max, y_min, color);
    swr_draw_line(buff, x_min, y_max, x_max, y_max, color);
    swr_draw_line(buff, x_min, y_min, x_min, y_max, color);
    swr_draw_line(buff, x_max, y_min, x_max, y_max, color);
}

#endif //  SWR_IMPLEMENTATION
