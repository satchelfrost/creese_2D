#include "creese_2D.h"

int main()
{
    init_window(500, 500, "audio test");
    init_audio_device();

    Sound sound = load_sound("assets/weird.wav");

    while (!window_should_close()) {
        if (RGFW_isKeyPressed(RGFW_space)) play_sound(sound);
        begin_drawing(BLUE);
        end_drawing();
    }

    unload_sound(sound);
    close_audio_device();
    close_window();
    return 0;
}
