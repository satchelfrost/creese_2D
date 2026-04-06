#include "creese_2D.h"

// TODO: this is highly experimental and not part of the API yet

#define STB_TRUETYPE_IMPLEMENTATION
#include "./external/stb_truetype.h"

#define NOB_STRIP_PREFIX
#include "../nob.h"

#define RGFWDEF extern
#include "./external/RGFW.h"

#define DEFAULT_BITMAP_WIDTH 180
#define DEFAULT_BITMAP_HEIGHT 170
#define FIRST_CHAR 32
#define CHAR_COUNT 96 

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

void draw_text_at_base(Font font, const char *text, size_t len, int x_start, int y_start, Color color)
{
    int x_adv = 0;
    for (size_t i = 0; i < len; i++) {
        uint8_t ch = text[i];
        if (!(FIRST_CHAR <= ch && ch < (CHAR_COUNT + FIRST_CHAR))) continue;
        Glyph glyph = font.glyphs[ch-FIRST_CHAR];
        int x, y;   // bitmap index
        int xp, yp; // pen position
        for (y = glyph.y0, yp = y_start; y <= glyph.y1; y++, yp++) {
            for (x = glyph.x0, xp = x_start; x <= glyph.x1; x++, xp++) {
                color.a = font.bitmap[y*DEFAULT_BITMAP_WIDTH + x];
                int coord_x = xp + glyph.x_offset + x_adv;
                int coord_y = yp + glyph.y_offset;
                if (color.a) draw_pixel(coord_x, coord_y, color);
            }
        }
        x_adv += glyph.x_advance;
    }
}

int main()
{
    Font font = load_font("assets/RobotoMono-Medium.ttf", 32);
    int screen_width = 1400;
    int screen_height = 250;
    init_window(screen_width, screen_height, "example text");

    const char *msg = "Hello, how are you doing today?";
    char all_chars[CHAR_COUNT];
    for (int i = 0; i < CHAR_COUNT; i++) all_chars[i] = FIRST_CHAR + i;
    int idx = 0;
    int width = font.glyphs[0].x_advance; // since this is monospace all xadvance values are the same

    while (!window_should_close()) {
        if (RGFW_isKeyPressed(RGFW_right)) idx++;
        if (RGFW_isKeyPressed(RGFW_left))  idx--;
        
        begin_drawing(WHITE);
            /* write a message on the first line (half height) */
            draw_line(0, screen_height/2, screen_width, screen_height/2, BLACK);
            draw_text_at_base(font, msg, strlen(msg), 0, screen_height/2.0, BLUE);
            Rectangle r = { .x = idx*width, .y = screen_height/2.0, .width = width, .height = 5, };
            draw_rectangle(r, RED);

            /* list all of the characters */
            draw_line(0, screen_height/2+font.height, screen_width, screen_height/2+font.height, BLACK);
            draw_text_at_base(font, all_chars, CHAR_COUNT, 0, screen_height/2.0+font.height, BLUE);
            r.y += font.height;
            draw_rectangle(r, RED);

        end_drawing();
    }

    close_window();

    return 0;
}
