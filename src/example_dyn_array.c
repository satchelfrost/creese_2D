#include "creese_2D.h"

#include "../nob.h"

typedef struct {
    int radius;
    V2f position;
    Color color;
} Circle;

typedef struct {
    /* common dynamic array structure */
    Circle *items;   // must have name items, but can point to any type
    size_t count;    // must have name count, and be size_t
    size_t capacity; // must have name capcity, and be size_t

    /* additional information not related to dynamic array */
    float timer;
} Circles;

float rand_float()
{
    return rand()/(float)RAND_MAX;
}

Color rand_color()
{
    return (Color) {
        .r = 255*rand_float(),
        .g = 255*rand_float(),
        .b = 255*rand_float(),
        .a = 255,
    };
}

int main()
{
    init_window(500, 500, "dynamic array");

    Circles circles = {0};
    Font font = load_font("assets/RobotoMono-Medium.ttf", 32);

    while (!window_should_close()) {
        /* input */
        if (mouse_inside_window()) {
            if (RGFW_isMousePressed(RGFW_mouseLeft))  {
                Mouse mouse = get_mouse_position();
                Circle circle = {
                    .radius = rand_float()*(10+9) + 9,
                    .color  = rand_color(),
                    .position = {.x = mouse.x, .y = mouse.y}
                };
                da_append(&circles, circle);
            }
        }

        circles.timer += get_frame_time();
        if (circles.timer > 10.0) {
            circles.count = 0; // no need to free memory we can just reset count
            circles.timer = 0;
        }

        /* drawing */
        begin_drawing(BLUE);
            for (size_t i = 0; i < circles.count; i++) {
                Circle c = circles.items[i];
                draw_circle(c.position.x, c.position.y, c.radius, c.color);
            }

            /* string building is also a dynamic array built into nob */
            String_Builder sb = {0};
            sb_appendf(&sb, "Timer:%d", 10 - (int)circles.timer);
            int padding = 10;
            draw_text_at_base(font, sb.items, sb.count, padding, font.height*2/3 + padding, BLACK);
            sb.count = 0; // reuse string builder memory
            if (circles.count) sb_appendf(&sb, "Circle count:%zu", circles.count);
            else               sb_appendf(&sb, "Click to add circles!");
            draw_text_at_base(font, sb.items, sb.count, padding, font.height*4/3 + padding, BLACK);
            sb.count = 0; // reuse string builder memory
            sb_appendf(&sb, "FPS:%d", get_avg_fps());
            draw_text_at_base(font, sb.items, sb.count, padding, font.height*6/3 + padding, BLACK);
        end_drawing();
    }

    close_window();
    return 0;
}
