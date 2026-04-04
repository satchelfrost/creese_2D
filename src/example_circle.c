#include "creese_2D.h"

int main()
{
    init_window(500, 500, "example circle");

    while (!window_should_close()) {
        begin_drawing(BLUE);
            draw_circle(250, 250, 50, RED);
        end_drawing();
    }

    close_window();
    return 0;
}
