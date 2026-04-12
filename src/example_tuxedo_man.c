#include "creese_2D.h"
#include <stdio.h>

enum {
    TUXEDO_ANIM_IDLE,
    TUXEDO_ANIM_RUN,
    TUXEDO_ANIM_JUMP,
    TUXEDO_ANIM_ATTACK,
    TUXEDO_ANIM_SWIM,
    TUXEDO_ANIM_SINGLE,
    TUXEDO_ANIM_COUNT,
};

enum {
    TUXEDO_SINGLE_FRAME_PARACHUTE,
    TUXEDO_SINGLE_FRAME_SHOCK,
    TUXEDO_SINGLE_FRAME_MARTINI,
    TUXEDO_SINGLE_FRAME_BOW,
    TUXEDO_SINGLE_FRAME_COUNT,
};

uint32_t anim_frame_count[TUXEDO_ANIM_COUNT] = {4, 6, 4, 3, 6, 4};

int main()
{
    int window_width = 500;
    int window_height= 500;
    init_window(window_width, window_height, "example tuxedo man");

    Image image = load_image("assets/Tuxedo.png");
    Image map = load_image("assets/club.png");
    if (!image.data) return 1;
    if (!map.data) return 1;

    float scale = 5.0;
    Sprite sprite = load_sprite_from_image(image, TUXEDO_ANIM_COUNT, TUXEDO_ANIM_COUNT, scale);

    /* these offsets deal with the fact that the sprite sheet has a black border around each sprite */
    sprite.sub_image.offset.x += 1;
    sprite.sub_image.offset.y += 1;
    sprite.sub_image.size.x -= 1;
    sprite.sub_image.size.y -= 1;

    V2f player_position = {.x = -5, .y = window_height/2.0f + 70 };
    int intro_sequence = 0;
    float sequence_timer = 0;
    float sequence_thresh = 3.0;

    while (!window_should_close()) {
        float dt = get_frame_time();

        while (true) {
            switch (intro_sequence) {
            case 0:
                player_position.x += dt*50;
                if (player_position.x >= window_width/2.0) {
                    intro_sequence = 1;
                    sprite.animation.frame = 0;
                    continue;
                }
                sprite.animation.type = TUXEDO_ANIM_RUN;
            break;
            case 1:
                if (sequence_timer > sequence_thresh) {
                    intro_sequence = 2;
                    sequence_timer = 0.0;
                    sprite.animation.frame = 0;
                    continue;
                }
                sequence_timer += dt;
                sprite.animation.playing = true;
                sprite.animation.type = TUXEDO_ANIM_IDLE;
            break;
            case 2:
                if (sequence_timer > sequence_thresh) {
                    intro_sequence = 1;
                    sequence_timer = 0.0;
                    sprite.animation.frame = 0;
                    continue;
                }
                sequence_timer += dt;
                sprite.animation.playing = false; // pause...
                sprite.animation.frame = TUXEDO_SINGLE_FRAME_MARTINI; // ...on this animation frame
                sprite.animation.type = TUXEDO_ANIM_SINGLE;
            break;
            };
            break;
        }

        if (sprite.animation.type == TUXEDO_ANIM_SINGLE)
            update_animation(&sprite, 1);
        else
            update_animation(&sprite, anim_frame_count[sprite.animation.type]);

        begin_drawing(BLUE);
            Rectangle city_rect = {
                .width = map.width/(float)3.0f,
                .height= map.height/(float)3.0f,
            };
            draw_image_rect_scaled(map, city_rect, window_width-2.0*city_rect.width, 0, 2.0, 2.0);
            draw_sprite_centered(sprite, player_position.x, player_position.y);
        end_drawing();
    }

    unload_image(image);
    close_window();
    return 0;
}
