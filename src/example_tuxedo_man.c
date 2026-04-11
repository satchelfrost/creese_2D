#include "creese_2D.h"

typedef enum {
    TUXEDO_ANIM_IDLE,
    TUXEDO_ANIM_RUN,
    TUXEDO_ANIM_JUMP,
    TUXEDO_ANIM_ATTACK,
    TUXEDO_ANIM_SWIM,
    TUXEDO_ANIM_SINGLE,
    TUXEDO_ANIM_COUNT,
} Tuxedo_Animation;

uint32_t anim_frame_count[TUXEDO_ANIM_COUNT] = {4, 6, 4, 3, 6, 4};

typedef struct {
    float scale;

    struct {
        bool playing;
        float time;
        float timeout;
        uint32_t frame;
        uint32_t type;
    } animation;

    Image image;
    struct {
        V2f size;
        V2f shift_amt;
        V2f offset;
        Rectangle rect;
    } sub_image;
} Sprite;

Rectangle get_anim_sub_rect(Sprite sprite)
{
    return (Rectangle) {
        .x      = sprite.sub_image.shift_amt.x*sprite.animation.type + sprite.sub_image.offset.x,
        .y      = sprite.sub_image.shift_amt.y*sprite.animation.frame + sprite.sub_image.offset.y,
        .width  = sprite.sub_image.size.x,
        .height = sprite.sub_image.size.y,
    };
}

void update_animation(Sprite *sprite, uint32_t total_anim_frames)
{
    if (sprite->animation.playing) {
        sprite->animation.time += get_frame_time();
        if (sprite->animation.time > sprite->animation.timeout) {
            sprite->animation.time = 0;
            sprite->animation.frame = (sprite->animation.frame + 1)%total_anim_frames;
            sprite->sub_image.rect = get_anim_sub_rect(*sprite);
        }
    }
}

#define DEFAULT_ANIMATION_TIMOUT (0.2f)

Sprite load_sprite_from_image(Image image, uint32_t horizontal_sprite_count, uint32_t vertical_sprite_count, float scale)
{
    Sprite sprite = {0};

    if (!image.data) {
        printf("ERROR: failed to load sprite from null image\n");
        return sprite;
    }

    float x = image.width/(float)horizontal_sprite_count;
    float y = image.height/(float)vertical_sprite_count;
    sprite.sub_image.shift_amt.x = x;
    sprite.sub_image.shift_amt.y = y;
    sprite.sub_image.size.x = x;
    sprite.sub_image.size.y = y;
    sprite.image = image;
    sprite.animation.playing = true;
    sprite.animation.timeout = DEFAULT_ANIMATION_TIMOUT;
    sprite.scale = (scale > 0) ? scale : 1.0f;

    return sprite;
}

void draw_sprite(Sprite sprite, int x, int y)
{
    draw_image_rect_scaled(sprite.image, sprite.sub_image.rect, x, y, sprite.scale, sprite.scale);
}

void draw_sprite_centered(Sprite sprite, int x, int y)
{
    x -= sprite.scale*sprite.sub_image.size.x/2.0f;
    y -= sprite.scale*sprite.sub_image.size.y/2.0f;
    draw_image_rect_scaled(sprite.image, sprite.sub_image.rect, x, y, sprite.scale, sprite.scale);
}

void draw_sprite_centered_debug(Sprite sprite, int x, int y)
{
    x -= sprite.scale*sprite.sub_image.size.x/2.0f;
    y -= sprite.scale*sprite.sub_image.size.y/2.0f;
    draw_image_rect_scaled(sprite.image, sprite.sub_image.rect, x, y, sprite.scale, sprite.scale);
    Rectangle r = {
        .x = x,
        .y = y,
        .width = sprite.sub_image.size.x*sprite.scale,
        .height = sprite.sub_image.size.y*sprite.scale,
    };
    draw_rectangle_lines(r, WHITE);
}

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
    sprite.sub_image.rect = get_anim_sub_rect(sprite);

    V2f player_position = {.x = -50, .y = window_height/2.0f + 70 };
    int intro_sequence = 0;
    float sequence_timer = 0;
    float sequence_thresh = 3.0;

    while (!window_should_close()) {
        float dt = get_frame_time();

        switch (intro_sequence) {
        case 0:
            player_position.x += dt*50;
            if (player_position.x >= window_width/2.0) intro_sequence = 1;
            sprite.animation.type = TUXEDO_ANIM_RUN;
        break;
        case 1:
            sequence_timer += dt;
            if (sequence_timer > sequence_thresh) {
                intro_sequence = 2;
                sequence_timer = 0.0;
            }
            sprite.animation.type = TUXEDO_ANIM_IDLE;
        break;
        case 2:
            sequence_timer += dt;
            if (sequence_timer > sequence_thresh) {
                intro_sequence = 1;
                sequence_timer = 0.0;
            }
            sprite.animation.type = TUXEDO_ANIM_SINGLE;
        break;
       }

        if (sprite.animation.type == TUXEDO_ANIM_SINGLE) {
            sprite.sub_image.rect = get_anim_sub_rect(sprite);
            sprite.animation.frame = 1; // pause on this animation frame
        } else {
            update_animation(&sprite, anim_frame_count[sprite.animation.type]);
        }

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
