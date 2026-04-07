#include "creese_2D.h"

#define RGFWDEF extern
#include "./external/RGFW.h"

int main()
{
    Creese_Font font = load_font("assets/RobotoMono-Medium.ttf", 32);
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
