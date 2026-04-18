#include "creese_2D.h"

enum {
    COLLISION_TEST_RECT_RECT,
    COLLISION_TEST_RECT_CIRCLE,
    COLLISION_TEST_CIRCLE_CIRCLE,
    COLLISION_TEST_COUNT,
} collision_test;

int main()
{
    int window_width = 500;
    int window_height = 500;
    init_window(window_width, window_height, "collision");

    Rectangle r0 = { .x = window_width/2.0f - 100, .y = window_height/2.0f - 25, .width = 200, .height= 50, };
    Rectangle r1 = { .width = 50, .height= 50, };
    V2f c0 = v2f(window_width/2.0f, window_height/2.0f);
    int c0_radius = 50;
    V2f c1 = v2f(window_width/2.0f, window_height/2.0f);
    int c1_radius = 50;
    bool collision = false;

    while (!window_should_close()) {
        /* input */
        Mouse mouse = get_mouse_position();
        r1.x = mouse.x - 25;
        r1.y = mouse.y - 25;
        c0.x = mouse.x;
        c0.y = mouse.y;
        if (RGFW_isMousePressed(RGFW_mouseLeft)) collision_test = (collision_test + 1)%COLLISION_TEST_COUNT;

        /* collision */
        switch (collision_test) {
        case COLLISION_TEST_RECT_RECT:
            collision = rectangle_collision(r0, r1);
        break;
        case COLLISION_TEST_RECT_CIRCLE:
            collision = rectangle_circle_collision(r0, c0.x, c0.y, c0_radius);
        break;
        case COLLISION_TEST_CIRCLE_CIRCLE:
            collision = circle_circle_collision(c0.x, c0.y, c0_radius, c1.x, c1.y, c1_radius);
        break;
        default: break;
        }

        /* drawing */
        begin_drawing(GRAY);
            switch (collision_test) {
            case COLLISION_TEST_RECT_RECT:
                draw_rectangle(r0, BLUE);
                draw_rectangle(r1, collision ? YELLOW : RED);
            break;
            case COLLISION_TEST_RECT_CIRCLE:
                draw_rectangle(r0, BLUE);
                draw_circle(c0.x, c0.y, c0_radius, collision ? YELLOW : RED);
            break;
            case COLLISION_TEST_CIRCLE_CIRCLE:
                draw_circle(c1.x, c1.y, c1_radius, BLUE);
                draw_circle(c0.x, c0.y, c0_radius, collision ? YELLOW : RED);
            break;
            default: break;
            }
        end_drawing();
    }

    close_window();
    return 0;
}
