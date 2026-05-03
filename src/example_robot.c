#include "creese_2D.h"
#include "../nob.h"

#define PLAYER_MOVE_SPEED 100
#define PROJECTILE_SPEED 400

enum {
    ROBOT_ANIM_IDLE,
    ROBOT_ANIM_RUN,
    ROBOT_ANIM_JUMP,
    ROBOT_ANIM_FIRE,
    ROBOT_ANIM_COUNT,
} animation;

const char *animations[] = {"idle", "run", "jump", "fire"};
int anim_frame_count[] = {9, 9, 7, 4};

struct {
    Image sprite_images[ROBOT_ANIM_COUNT];
    Sprite sprites[ROBOT_ANIM_COUNT];
} robot;

int main()
{
    init_window(500, 500, "robot");

    for (int i = 0; i < ROBOT_ANIM_COUNT; i++) {
        robot.sprite_images[i] = load_image(temp_sprintf("assets/super-cool-robot-%s.png", animations[i]));
        robot.sprites[i] = load_sprite_from_image(robot.sprite_images[i], anim_frame_count[i], 0, 1.0);
        robot.sprites[i].animation.horizontal = true;
        robot.sprites[i].animation.timeout = 0.1; // might want to be vary across animations
    }

    V2f pos = {.x = 0, .y = 250-robot.sprites[0].sub_image.size.y}; // same for all sprites?
    int x_dir = 1;
    bool firing = false;
    float firing_time = 0;
    V2f projectile = pos;
    bool projectile_active = false;
    int projectile_dir = 1;

    while (!window_should_close()) {
        float dt = get_frame_time();
        bool moving = false;

        if (is_key_down(KEY_RIGHT)) {
            update_animation(&robot.sprites[ROBOT_ANIM_RUN], anim_frame_count[ROBOT_ANIM_RUN]);
            x_dir = 1;
            pos.x += x_dir*dt*PLAYER_MOVE_SPEED;
            moving = true;
        }
        if (is_key_down(KEY_LEFT)) {
            update_animation(&robot.sprites[ROBOT_ANIM_RUN], anim_frame_count[ROBOT_ANIM_RUN]);
            x_dir = -1;
            pos.x += x_dir*dt*PLAYER_MOVE_SPEED;
            moving = true;
        }
        update_animation(&robot.sprites[ROBOT_ANIM_IDLE], anim_frame_count[ROBOT_ANIM_IDLE]);

        if (is_key_pressed(KEY_SPACE)) {
            firing = true;
            projectile.x = pos.x + robot.sprites[ROBOT_ANIM_FIRE].sub_image.size.x/2 + x_dir*35;
            projectile.y = pos.y + robot.sprites[ROBOT_ANIM_FIRE].sub_image.size.y/2 + 20;
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
            projectile.x += projectile_dir*dt*PROJECTILE_SPEED;
            if (!(0 < projectile.x && projectile.x < 500-1))
                projectile_active = false;
        }

        begin_drawing(BLUE);
            if (moving) {
                if (x_dir > 0) draw_sprite(robot.sprites[ROBOT_ANIM_RUN], pos.x, pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_RUN], pos.x, pos.y);
            } else if (firing) {
                if (x_dir > 0) draw_sprite(robot.sprites[ROBOT_ANIM_FIRE], pos.x, pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_FIRE], pos.x, pos.y);
            } else {
                if (x_dir > 0) draw_sprite(robot.sprites[ROBOT_ANIM_IDLE], pos.x, pos.y);
                else           draw_sprite_flip_x(robot.sprites[ROBOT_ANIM_IDLE], pos.x, pos.y);
            }
            if (projectile_active) {
                draw_circle(projectile.x, projectile.y, 10, YELLOW);
            }
            draw_line(0, 250, 500, 250, BLACK);
        end_drawing();
    }

    close_window();
    return 0;
}
