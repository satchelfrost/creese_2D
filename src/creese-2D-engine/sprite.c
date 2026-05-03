#include "../creese_2D.h"

#define DEFAULT_ANIMATION_TIMOUT (0.2f)

Rectangle get_anim_sub_rect(Sprite sprite)
{
    /* sprite animation frames either move across a row (horizontal) or across a column */
    uint32_t x_idx = (sprite.animation.horizontal) ? sprite.animation.frame : sprite.animation.type;
    uint32_t y_idx = (sprite.animation.horizontal) ? sprite.animation.type  : sprite.animation.frame;

    return (Rectangle) {
        .x      = sprite.sub_image.shift_amt.x*x_idx + sprite.sub_image.offset.x,
        .y      = sprite.sub_image.shift_amt.y*y_idx + sprite.sub_image.offset.y,
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
        }
    }

    /* always recompute animation sub rect in case sub image changed */
    sprite->sub_image.rect = get_anim_sub_rect(*sprite);
}

Sprite load_sprite_from_image(Image image, uint32_t horizontal_sprite_count, uint32_t vertical_sprite_count, float scale)
{
    Sprite sprite = {0};

    if (!image.data) {
        printf("ERROR: failed to load sprite from null image\n");
        return sprite;
    }

    horizontal_sprite_count = (horizontal_sprite_count) ? horizontal_sprite_count : 1;
    vertical_sprite_count   = (vertical_sprite_count)   ? vertical_sprite_count   : 1;

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
    sprite.sub_image.rect = get_anim_sub_rect(sprite);

    return sprite;
}

void draw_sprite(Sprite sprite, int x, int y)
{
    draw_image_rect_scaled(sprite.image, sprite.sub_image.rect, x, y, sprite.scale, sprite.scale);
}

void draw_sprite_flip_x(Sprite sprite, int x, int y)
{
    draw_image_rect_scaled_flip_x(sprite.image, sprite.sub_image.rect, x, y, sprite.scale, sprite.scale);
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
