#include "creese_2D.h"
#include <string.h>

#define RETICLE_MOVE_SPEED 500
#define PLAYER_MOVE_SPEED 10
#define PROJECTILE_SPEED 900
#define MAX_PROJECTILES 5 
#define MAX_BOUNCES 10

struct {
    bool active;
    V2f pos;
    V2f velocity;
    int bounce_count;
} projectiles[MAX_PROJECTILES] = {0};

#define BRICK_ROWS 4
#define BRICK_COLS 10

struct {
    bool active;
    Rectangle rect;
} bricks[BRICK_ROWS*BRICK_COLS];

void init_bricks(int window_width)
{
    float padding = 10;
    float width = (window_width - 2*padding)/(float)BRICK_COLS;
    float height = width/2.0f;
    for (int y = 0; y < BRICK_ROWS; y++) {
        for (int x = 0; x < BRICK_COLS; x++) {
            bricks[y*BRICK_COLS + x].rect.x = padding + width*x;
            bricks[y*BRICK_COLS + x].rect.y = padding + height*y;
            bricks[y*BRICK_COLS + x].rect.width = width;
            bricks[y*BRICK_COLS + x].rect.height = height;
            bricks[y*BRICK_COLS + x].active = true;
        }
    }
}

enum {
    GAME_STATE_ON,
    GAME_STATE_OVER,
    GAME_STATE_WON,
} game_state = 0;

int main()
{
    int window_width = 500;
    int window_height = 800;
    init_window(window_width, window_height, "brick 'em shmup");
    init_audio_device();

    float reticle_size = 5;
    float projectile_size = reticle_size;
    V2f reticle_pos = v2f(window_width/2, window_height - 100);
    float player_reticle_padding = 50;
    V2f player_pos = v2f(reticle_pos.x, reticle_pos.y + player_reticle_padding);
    float player_size = 15;
    bool firing = false;
    init_bricks(window_width);
    Font font = load_font("assets/RobotoMono-Medium.ttf", 32);

    while (!window_should_close()) {
        float dt = get_frame_time();

        if (is_key_down(KEY_LEFT)) {
            float nx = reticle_pos.x - dt*RETICLE_MOVE_SPEED;
            if (0 < nx - player_size && nx + player_size < window_width  - 1) reticle_pos.x = nx;
        }
        if (is_key_down(KEY_RIGHT)) {
            float nx = reticle_pos.x + dt*RETICLE_MOVE_SPEED;
            if (0 < nx - player_size && nx + player_size < window_width -  1) reticle_pos.x = nx;
        }

        if (is_key_pressed(KEY_SPACE)) {
            firing = true;
        } else {
            firing = false;
        }
        if (is_key_pressed(KEY_R)) {
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                projectiles[i].active = false;
            }
            for (int i = 0; i < BRICK_ROWS*BRICK_COLS; i++) {
                bricks[i].active = true;
            }
            game_state = GAME_STATE_ON;
        }

        /* player lazy follow */
        float dx = reticle_pos.x - player_pos.x;
        Color player_color = RED;
        if (fabsf(dx) < 1) {
            player_pos.x = reticle_pos.x;
            player_color = BLUE;
        }
        else {
           player_pos.x += dt*dx*PLAYER_MOVE_SPEED;
           player_color = RED;
        }
        player_pos.y = reticle_pos.y + player_reticle_padding;

        /* projectile spawn */
        if (firing) {
            int projectile_idx = -1;
            for (int i = 0; i < MAX_PROJECTILES; i++) {
                if (!projectiles[i].active) {
                    projectiles[i].active = true;
                    projectile_idx = i;
                    break;
                }
            }
            if (projectile_idx != -1) {
                projectiles[projectile_idx].pos = reticle_pos;
                projectiles[projectile_idx].velocity = v2f_norm(v2f_sub(reticle_pos, player_pos), 0.001f, v2f(0.0, 0.0));
                projectiles[projectile_idx].bounce_count = MAX_BOUNCES;
            }
        }

        /* projectile physics */
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (projectiles[i].active) {
                float nx = projectiles[i].pos.x + projectiles[i].velocity.x*dt*PROJECTILE_SPEED;
                float ny = projectiles[i].pos.y + projectiles[i].velocity.y*dt*PROJECTILE_SPEED;
                if (0 < nx - projectile_size && nx + projectile_size < window_width  - 1)
                    projectiles[i].pos.x = nx;
                else {
                    projectiles[i].velocity.x *= -1.0;
                    projectiles[i].bounce_count -= 1;
                    if (projectiles[i].bounce_count <= 0)
                        projectiles[i].active = false;
                }

                if (0 < ny - projectile_size && ny + projectile_size < window_height - 1)
                    projectiles[i].pos.y = ny;
                else {
                    projectiles[i].velocity.y *= -1.0;
                    projectiles[i].bounce_count -= 1;
                    if (projectiles[i].bounce_count <= 0)
                        projectiles[i].active = false;
                }
            }
        }

        /* bricks */
        int brick_active_count = 0;
        for (int i = 0; i < BRICK_ROWS*BRICK_COLS; i++) {
            if (!bricks[i].active) continue;
            brick_active_count++;
            for (int j = 0; j < MAX_PROJECTILES; j++) {
                if (!projectiles[j].active) continue;

                if (rectangle_circle_collision(bricks[i].rect, projectiles[j].pos.x, projectiles[j].pos.y, projectile_size)) {
                    bricks[i].active = false;
                    projectiles[j].velocity.y *= -1.0;
                }
            }
        }
        if (!brick_active_count) game_state = GAME_STATE_WON;

        /* player collision */
        for (int i = 0; i < MAX_PROJECTILES; i++) {
            if (!projectiles[i].active) continue;
            if (circle_circle_collision(projectiles[i].pos.x, projectiles[i].pos.y, projectile_size, player_pos.x, player_pos.y, player_size))
                game_state = GAME_STATE_OVER;
        }


        begin_drawing(BLACK);
            if (game_state == GAME_STATE_ON) {
                draw_circle(reticle_pos.x, reticle_pos.y, reticle_size, YELLOW);
                draw_circle(player_pos.x, player_pos.y, player_size, player_color);

                for (int i = 0; i < MAX_PROJECTILES; i++) {
                    if (projectiles[i].active) {
                        draw_circle(projectiles[i].pos.x, projectiles[i].pos.y, projectile_size, PURPLE);
                    }
                }

                for (int i = 0; i < BRICK_ROWS*BRICK_COLS; i++) {
                    if (!bricks[i].active) continue;
                    draw_rectangle(bricks[i].rect, PINK);
                    draw_rectangle_lines(bricks[i].rect, BLACK);
                }
            } else if (game_state == GAME_STATE_WON) {
                const char *won = "Game Won!!";
                draw_text_at_base(font, won, strlen(won), 180, window_height/2, WHITE);
            } else if (game_state == GAME_STATE_OVER) {
                const char *game_over = "Game Over!";
                draw_text_at_base(font, game_over, strlen(game_over), 180, window_height/2, WHITE);
            }
        end_drawing();
    }

    close_window();
    return 0;
} 
