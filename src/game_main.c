#include "creese_2D.h"

/* Note this file doesn't do anything right now, but
 * just serves as an example for how we will add new C files.
 * This style of include-c-file-directly is called a unity build.
 * Any new c includes do need to update nob.c's game_dep_srcs[] */
#include "game_example_dependency.c"

int main()
{
    int window_width = 500;
    int window_height = 500;
    init_window(window_width, window_height, "Creese 2D First Ever Jam!");

    while (!window_should_close()) {
        begin_drawing(WHITE);
            draw_circle(window_width/2, window_height/2, 50, RED);
        end_drawing();
    }

    close_window();
    return 0;
}
