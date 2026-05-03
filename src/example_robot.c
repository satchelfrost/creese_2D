#include "creese_2D.h"
#include "../nob.h"

#define PLAYER_MOVE_SPEED 100
#define JUMP_SPEED 500
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

struct {
    Image sprite_images[ROBOT_ANIM_COUNT];
    Sprite sprites[ROBOT_ANIM_COUNT];
} robot;

// TODO: new_state = handle_action_state(current_state) // action_state e.g. jump, fire, etc.
// TODO: order things better i.e. input, then physics/character state, then animation,
// TODO: define position based on bottom center, then calculate top left before rendering

enum {
    MOVEMENT_NONE,
    MOVEMENT_RIGHT,
    MOVEMENT_LEFT,
};

int main()
{
    init_window(500, 500, "robot");
    init_audio_device();

    for (int i = 0; i < ROBOT_ANIM_COUNT; i++) {
        robot.sprite_images[i] = load_image(temp_sprintf("assets/super-cool-robot-%s.png", animations[i]));
        robot.sprites[i] = load_sprite_from_image(robot.sprite_images[i], anim_frame_count[i], 0, 1.0);
        robot.sprites[i].animation.horizontal = true;
        robot.sprites[i].animation.timeout = 0.1; // might want to be vary across animations
    }

    float floor = 250;
    V2f pos = {.x = 0, .y = 250-robot.sprites[0].sub_image.size.y}; // same for all sprites?
    int x_dir = 1;
    bool firing = false;
    float firing_time = 0;
    V2f projectiles[4];
    bool projectile_active = false;
    int projectile_dir = 1;
    bool jumping = false;
    V2f velocity = {0};
    float y_acceleration = 0;
    int movement = MOVEMENT_NONE;
    Sound gun_fire = load_sound("assets/weird.wav");

    while (!window_should_close()) {
        float dt = get_frame_time();

        if (is_key_down(KEY_RIGHT) && !jumping) {
            update_animation(&robot.sprites[ROBOT_ANIM_RUN], anim_frame_count[ROBOT_ANIM_RUN]);
            x_dir = 1;
            pos.x += x_dir*dt*PLAYER_MOVE_SPEED;
            movement = MOVEMENT_RIGHT;
        } else if (is_key_down(KEY_LEFT) && !jumping) {
            update_animation(&robot.sprites[ROBOT_ANIM_RUN], anim_frame_count[ROBOT_ANIM_RUN]);
            x_dir = -1;
            pos.x += x_dir*dt*PLAYER_MOVE_SPEED;
            movement = MOVEMENT_RIGHT;
        } else movement = MOVEMENT_NONE;
        update_animation(&robot.sprites[ROBOT_ANIM_IDLE], anim_frame_count[ROBOT_ANIM_IDLE]);
        if (is_key_pressed(KEY_UP) && !jumping) {
            jumping = true;
            velocity.y = -dt*JUMP_SPEED;
            velocity.x = (movement) ? dt*PLAYER_MOVE_SPEED*x_dir : 0;
            y_acceleration = 10*dt;
        }

        if (is_key_pressed(KEY_SPACE)) {
            firing = true;
            play_sound(gun_fire);
            for (int i = 0; i < 4; i++) {
                projectiles[i].y = pos.y + robot.sprites[ROBOT_ANIM_FIRE].sub_image.size.y/2 + 20;
                projectiles[i].x = pos.x + robot.sprites[ROBOT_ANIM_FIRE].sub_image.size.x/2 + x_dir*35 + 20*i*x_dir;
            }
            projectile_active = true;
            projectile_dir = x_dir;
        }

        if (firing) {
            update_animation(&robot.sprites[ROBOT_ANIM_FIRE], anim_frame_count[ROBOT_ANIM_FIRE]);
            firing_time += dt;
            if (firing_time > 0.3) {
                firing_time = 0;
                firing = false;
            }
        } else {
           robot.sprites[ROBOT_ANIM_FIRE].animation.frame = 0;
        }

        if (projectile_active) {
            for (int i = 0; i < 4; i++) {
                projectiles[i].x += projectile_dir*dt*PROJECTILE_SPEED;
            }
            if (!(0 < projectiles[0].x && projectiles[0].x < 500-1))
                projectile_active = false;
        }

        if (jumping) {
            if (robot.sprites[ROBOT_ANIM_JUMP].animation.frame == anim_frame_count[ROBOT_ANIM_JUMP]) {
                robot.sprites[ROBOT_ANIM_JUMP].animation.playing = false;
            }
            update_animation(&robot.sprites[ROBOT_ANIM_JUMP], anim_frame_count[ROBOT_ANIM_JUMP]);
            velocity.y += y_acceleration;
            pos.x += velocity.x;
            pos.y += velocity.y;
            if (pos.y + robot.sprites[0].sub_image.size.y > floor) {
                jumping = false;
                pos.y = floor - robot.sprites[0].sub_image.size.y;
                robot.sprites[ROBOT_ANIM_JUMP].animation.playing = true;
                robot.sprites[ROBOT_ANIM_JUMP].animation.frame = 0;
            }

        }

        begin_drawing(BLUE);
            if (movement) {
                if (x_dir > 0) draw_sprite(robot.sprites[ROBOT_ANIM_RUN], pos.x, pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_RUN], pos.x, pos.y);
            } else if (firing) {
                if (x_dir > 0) draw_sprite(robot.sprites[ROBOT_ANIM_FIRE], pos.x, pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_FIRE], pos.x, pos.y);
            } else if (jumping) {
                if (x_dir > 0) draw_sprite(robot.sprites[ROBOT_ANIM_JUMP], pos.x, pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_JUMP], pos.x, pos.y);
            } else {
                if (x_dir > 0) draw_sprite(robot.sprites[ROBOT_ANIM_IDLE], pos.x, pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_IDLE], pos.x, pos.y);
            }
            if (projectile_active) {
                for (int i = 0; i < 4; i++)
                    draw_circle(projectiles[i].x, projectiles[i].y, 10, YELLOW);
            }
            draw_line(0, 250, 500, 250, BLACK);
        end_drawing();
    }

    close_window();
    return 0;
}
