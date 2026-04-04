#include "creese_2D.h"

int main()
{
    int window_width = 500;
    int window_height = 500;
    init_window(window_width, window_height, "example image");

    Image image = load_image("assets/magic_button_blue.png");
    V2i image_pos = {
        .x = window_width/2.0f - image.width/2.0f,
        .y = window_height/2.0f - image.height/2.0f,
    };

    while (!window_should_close()) {
        begin_drawing(RED);
            draw_image(image, image_pos.x, image_pos.y);
        end_drawing();
    }

    unload_image(image);
    close_window();
    return 0;
}
