#include "creese_2D.h"
#include "../nob.h"

#define PLAYER_TRANSLATE_SPEED 100
#define PLAYER_JUMP_SPEED 500
#define PROJECTILE_SPEED 400

enum {
    ROBOT_ANIM_IDLE,
    ROBOT_ANIM_RUN,
    ROBOT_ANIM_JUMP,
    ROBOT_ANIM_FIRE,
    ROBOT_ANIM_COUNT,
} animation;

const char *animations[] = {"idle", "run", "jump", "fire"};
uint32_t anim_frame_count[] = {9, 9, 7, 4};

#define MAX_PROJECTILES 4

typedef struct {
    V2f pos;
    V2f velocity;
    bool active;
} Projectile;

struct {
    V2f pos;
    V2f render_pos;
    V2f velocity;
    bool jumping;
    float gravity;
    int movement;
    int facing_direction;

    Image sprite_images[ROBOT_ANIM_COUNT];
    Sprite sprites[ROBOT_ANIM_COUNT];

    struct {
        bool play_firing_animation;
        float firing_animation_time;
        Sound fire_sound;
        Projectile projectiles[MAX_PROJECTILES];
    } gun;
} robot;

void load_player(int ground)
{
    /* load sprites */
    for (int i = 0; i < ROBOT_ANIM_COUNT; i++) {
        robot.sprite_images[i] = load_image(temp_sprintf("assets/super-cool-robot-%s.png", animations[i]));
        robot.sprites[i] = load_sprite_from_image(robot.sprite_images[i], anim_frame_count[i], 0, 1.0);
        robot.sprites[i].animation.horizontal = true;
        robot.sprites[i].animation.timeout = 0.1; // might want to be vary across animations
    }
    robot.pos = v2f(robot.sprites[0].sub_image.size.x/2.0f, ground);
    robot.facing_direction = 1;
    robot.gun.fire_sound = load_sound("assets/weird.wav");
}

V2f render_pos_from_bottom_center(V2f bottom_center, Sprite sprite)
{
    return v2f(bottom_center.x - sprite.sub_image.size.x/2.0f,
               bottom_center.y - sprite.sub_image.size.y);
}

// TODO: new_state = handle_action_state(current_state) // action_state e.g. jump, fire, etc.
// TODO: order things better i.e. input, then physics/character state, then animation,
// TODO: have less of a zoo of draw sprite functions, for example the basic draw sprite should have flip_x and tint
// TODO: some sort of bug with window focus? Whenever I get an OS notification (e.g. low battery) weird shit happens
// TODO: firing doesn't feel great, maybe is_key_down(KEY_SPACE)? But we'll need a firing timout.

enum {
    MOVEMENT_NONE,
    MOVEMENT_RIGHT,
    MOVEMENT_LEFT,
};

int main()
{
    int window_width = 500;
    int window_height = 500;
    init_window(window_width, window_height, "robot");
    init_audio_device();
    Font font = load_font("assets/RobotoMono-Medium.ttf", 32);

    int ground = window_height/2.0f;
    load_player(ground);
    bool display_fps = true;
    String_Builder sb = {0};

    while (!window_should_close()) {
        float dt = get_frame_time();

        if (is_key_pressed(KEY_F)) display_fps = !display_fps;
        if (is_key_down(KEY_RIGHT) && !robot.jumping) {
            update_animation(&robot.sprites[ROBOT_ANIM_RUN], anim_frame_count[ROBOT_ANIM_RUN]);
            robot.facing_direction = 1;
            robot.movement = MOVEMENT_RIGHT;
            robot.pos.x += robot.facing_direction*dt*PLAYER_TRANSLATE_SPEED;
        } else if (is_key_down(KEY_LEFT) && !robot.jumping) {
            update_animation(&robot.sprites[ROBOT_ANIM_RUN], anim_frame_count[ROBOT_ANIM_RUN]);
            robot.facing_direction = -1;
            robot.pos.x += robot.facing_direction*dt*PLAYER_TRANSLATE_SPEED;
            robot.movement = MOVEMENT_RIGHT;
        } else robot.movement = MOVEMENT_NONE;
        update_animation(&robot.sprites[ROBOT_ANIM_IDLE], anim_frame_count[ROBOT_ANIM_IDLE]);
        if (is_key_pressed(KEY_UP) && !robot.jumping) {
            robot.jumping = true;
            robot.velocity.y = -dt*PLAYER_JUMP_SPEED;
            robot.velocity.x = (robot.movement) ? dt*PLAYER_TRANSLATE_SPEED*robot.facing_direction : 0;
            robot.gravity = 10*dt;
        }

        if (is_key_pressed(KEY_SPACE)) {
            robot.sprites[ROBOT_ANIM_FIRE].animation.frame = 0;
            robot.sprites[ROBOT_ANIM_FIRE].animation.time = 0;
            for (int i = 0; i < 4; i++) {
                if (robot.gun.projectiles[i].active) continue;

                /* only play firing animation and sound if we've found inactive projectile to reuse */
                robot.gun.play_firing_animation = true;
                // play_sound(robot.gun.fire_sound);

                robot.gun.projectiles[i].pos.y = robot.pos.y - 25;
                robot.gun.projectiles[i].pos.x = robot.pos.x + robot.facing_direction*30;
                robot.gun.projectiles[i].active = true;
                robot.gun.projectiles[i].velocity = v2f(robot.facing_direction*PROJECTILE_SPEED, 0);
                break; // since we've found at least one projectile to activate
            }
        }

        if (robot.gun.play_firing_animation) {
            update_animation(&robot.sprites[ROBOT_ANIM_FIRE], anim_frame_count[ROBOT_ANIM_FIRE]);
            robot.gun.firing_animation_time += dt;
            if (robot.gun.firing_animation_time > 0.3) {
                robot.gun.firing_animation_time = 0;
                robot.gun.play_firing_animation = false;
            }
        }

        /* projeatile physics */
        for (int i = 0; i < 4; i++) {
            if (!robot.gun.projectiles[i].active) continue;

            float nx = robot.gun.projectiles[i].pos.x + dt*robot.gun.projectiles[i].velocity.x;
            if (0 < nx && nx < window_width) robot.gun.projectiles[i].pos.x = nx;
            else                             robot.gun.projectiles[i].active = false;
        }

        if (robot.jumping) {
            if (robot.sprites[ROBOT_ANIM_JUMP].animation.frame == anim_frame_count[ROBOT_ANIM_JUMP]) {
                robot.sprites[ROBOT_ANIM_JUMP].animation.playing = false;
            }
            update_animation(&robot.sprites[ROBOT_ANIM_JUMP], anim_frame_count[ROBOT_ANIM_JUMP]);
            robot.velocity.y += robot.gravity;
            robot.pos.x += robot.velocity.x;
            robot.pos.y += robot.velocity.y;
            if (robot.pos.y > ground) {
                robot.jumping = false;
                robot.pos.y = ground;
                robot.sprites[ROBOT_ANIM_JUMP].animation.playing = true;
                robot.sprites[ROBOT_ANIM_JUMP].animation.frame = 0;
            }

        }

        begin_drawing(BLUE);
            robot.render_pos = render_pos_from_bottom_center(robot.pos, robot.sprites[0]);
            if (robot.movement) {
                if (robot.facing_direction > 0) draw_sprite(robot.sprites[ROBOT_ANIM_RUN], robot.render_pos.x, robot.render_pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_RUN], robot.render_pos.x, robot.render_pos.y);
            } else if (robot.gun.play_firing_animation) {
                if (robot.facing_direction > 0) draw_sprite(robot.sprites[ROBOT_ANIM_FIRE], robot.render_pos.x, robot.render_pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_FIRE], robot.render_pos.x, robot.render_pos.y);
            } else if (robot.jumping) {
                if (robot.facing_direction > 0) draw_sprite(robot.sprites[ROBOT_ANIM_JUMP], robot.render_pos.x, robot.render_pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_JUMP], robot.render_pos.x, robot.render_pos.y);
            } else {
                if (robot.facing_direction > 0) draw_sprite(robot.sprites[ROBOT_ANIM_IDLE], robot.render_pos.x, robot.render_pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_IDLE], robot.render_pos.x, robot.render_pos.y);
            }
            for (int i = 0; i < 4; i++) {
                if (!robot.gun.projectiles[i].active) continue;
                draw_circle(robot.gun.projectiles[i].pos.x, robot.gun.projectiles[i].pos.y, 10, YELLOW);
            }
            Rectangle ground_rect = {.x = 0, .y = ground, .width = window_width, .height = ground};
            draw_rectangle(ground_rect, GREEN);

            if (display_fps) {
                sb.count = 0;
                int padding = 10;
                sb_appendf(&sb, "FPS:%d", get_avg_fps());
                draw_text_at_base(font, sb.items, sb.count, padding, font.height*2/3 + padding, BLACK);
            }
        end_drawing();
    }

    unload_font(font);
    sb_free(sb);
    close_window();
    return 0;
}
