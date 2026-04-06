#include "creese_2D.h"

// TODO: this is highly experimental and not part of the API yet

#define STB_TRUETYPE_IMPLEMENTATION
#include "./external/stb_truetype.h"
// #include "../file.txt"

#define NOB_STRIP_PREFIX
#include "../nob.h"

#define RGFWDEF extern
#include "./external/RGFW.h"

#define BITMAP_WIDTH 180
#define BITMAP_HEIGHT 170
#define FIRST_CHAR 32
#define CHAR_COUNT 96 

unsigned char bitmap[BITMAP_WIDTH*BITMAP_HEIGHT];
stbtt_bakedchar cdata[CHAR_COUNT]; // ASCII 32..126 is 95 glyphs

void draw_text_at_base(const char *text, size_t len, int x_start, int y_start)
{
    int adv = 0.0f;
    for (size_t i = 0; i < len; i++) {
        uint8_t character = text[i]-FIRST_CHAR;
        if (!(character < CHAR_COUNT)) continue;
        stbtt_bakedchar c = cdata[text[i]-FIRST_CHAR];
        int x, y, xp, yp;
        for (y = c.y0, yp = y_start; y <= c.y1; y++, yp++) {
            for (x = c.x0, xp = x_start; x <= c.x1; x++, xp++) {
                Color color = BLUE;
                color.a = ((uint8_t*)bitmap)[y*BITMAP_WIDTH + x];
                if (color.a) draw_pixel(xp+c.xoff + adv, yp+c.yoff, color);
            }
        }
        adv += c.xadvance;
    }
}

int main()
{
    String_Builder sb = {0};
    const char *file = "assets/RobotoMono-Medium.ttf";
    if (!read_entire_file(file, &sb)) return 1;
    float pixel_height = 32.0f;
    stbtt_BakeFontBitmap((unsigned char *)sb.items, 0, pixel_height, bitmap,
                         BITMAP_WIDTH, BITMAP_HEIGHT, FIRST_CHAR, CHAR_COUNT, cdata);

    int screen_width = 1400;
    int screen_height = 250;
    init_window(screen_width, screen_height, "example text");
    const char *msg = "Hello, how are you doing today?";
    // const char *msg = "how are you doing today?";
    // const char *msg2 = "how are you doing today?";

    int idx = 0;
    int width = cdata[0].xadvance;
    while (!window_should_close()) {
        if (RGFW_isKeyPressed(RGFW_right)) idx++;
        if (RGFW_isKeyPressed(RGFW_left)) idx--;
        begin_drawing(WHITE);
            draw_text_at_base(msg, strlen(msg), 0, screen_height/2.0);

            V2i start = {{0, screen_height/2}};
            V2i end   = {{1400, screen_height/2}};
            draw_line(start, end, BLACK);
            Rectangle r = {
                .x = idx*width,
                .y = screen_height/2.0-pixel_height,
                .width = width,
                .height = pixel_height,
            };
            Color rc = RED;
            // rc.a = 50;
            draw_rectangle(r, rc);

        end_drawing();
    }

    close_window();

    return 0;
}
