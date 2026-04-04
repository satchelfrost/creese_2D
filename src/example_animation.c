#include "creese_2D.h"

int main()
{
    int window_width = 500;
    int window_height= 500;
    init_window(window_width, window_height, "example animation");
    
    Image image = load_image("assets/flames.png");
    int anim_frame_count = 7; // this number we figure out from the sprite sheet
    int anim_frame = 0;
    float anim_timeout = 0.1f;
    float anim_time = 0.0f;
    float image_scale = 10.0f;

    /* rectangle is the sub section of the sprite sheet */
    Rectangle rect = {
        .width  = image.width / anim_frame_count,
        .height = image.height,
    };

    while (!window_should_close()) {
        anim_time += get_frame_time();
        if (anim_time > anim_timeout) {
            anim_time = 0;
            anim_frame = (anim_frame + 1)%anim_frame_count;
            rect.x = anim_frame*rect.width; // offset in the sprite sheet
        }

        int sprite_pos_x = window_width/2.0f - rect.width*image_scale/2.0f;
        int sprite_pos_y = window_height/2.0f - rect.height*image_scale/2.0f;

        begin_drawing(GRAY);
            draw_image_rect_scaled(image, rect, sprite_pos_x, sprite_pos_y, image_scale, image_scale);
        end_drawing();
    }

    close_window();
    return 0;
}
