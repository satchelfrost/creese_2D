#include "creese_2D.h"

// TODO: this is highly experimental and not part of the API yet

#define STB_TRUETYPE_IMPLEMENTATION
#include "./external/stb_truetype.h"
// #include "../file.txt"

#define NOB_STRIP_PREFIX
#include "../nob.h"

#define RGFWDEF extern
#include "./external/RGFW.h"

#define BITMAP_WIDTH 400
#define BITMAP_HEIGHT BITMAP_WIDTH
#define FIRST_CHAR 32
#define CHAR_COUNT 96 

int main()
{
    String_Builder sb = {0};
    const char *file = "assets/RobotoMono-Medium.ttf";
    if (!read_entire_file(file, &sb)) return 1;
    float pixel_height = 32.0f;
    unsigned char bitmap[BITMAP_WIDTH*BITMAP_HEIGHT];
    stbtt_bakedchar cdata[CHAR_COUNT]; // ASCII 32..126 is 95 glyphs
    stbtt_BakeFontBitmap((unsigned char *)sb.items, 0, pixel_height, bitmap,
                         BITMAP_WIDTH, BITMAP_HEIGHT, FIRST_CHAR, CHAR_COUNT, cdata);

    int screen_width = 1400;
    int screen_height = 250;
    init_window(screen_width, screen_height, "example text");

    while (!window_should_close()) {
        begin_drawing(WHITE);
            for (size_t i = 0; i < CHAR_COUNT; i++) {
                stbtt_bakedchar c = cdata[i];
                int x, y, xp, yp;
                for (y = c.y0, yp = screen_height/2; y < c.y1; y++, yp++) {
                    for (x = c.x0, xp = 0; x < c.x1; x++, xp++) {
                        Color color = BLUE;
                        color.a = ((uint8_t*)bitmap)[y*BITMAP_WIDTH + x];
                        if (color.a) draw_pixel(xp+c.xoff + i*c.xadvance, yp+c.yoff, color);
                    }
                }
            }

            V2i start = {{0, screen_height/2}};
            V2i end   = {{1400, screen_height/2}};
            draw_line(start, end, BLACK);
        end_drawing();
    }

    close_window();

    return 0;
}
