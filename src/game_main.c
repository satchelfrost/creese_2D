#include "creese_2D.h"

#include "../nob.h"

/* Note this file doesn't do anything right now, but
 * just serves as an example for how we will add new C files.
 * This style of include-c-file-directly is called a unity build.
 * Any new c includes do need to update nob.c's game_dep_srcs[] */
#include "game_example_dependency.c"

#define DEBUG 1

// TODO: zelda unlock sound analogue
// TODO: success music
// TODO: walk sound

typedef enum {
    ELEMENT_NONE,
    ELEMENT_FIRE,
    ELEMENT_WATER,
    ELEMENT_LIGHTNING,
    ELEMENT_COUNT,
} Element;

typedef struct {
    Element element;
    V2f position;
    bool pressed;
    Image image;
    float scale;

    struct {
        Image image;
        Sprite sprite;
        float scale;
        Sound sound;
    } flame;

    struct {
        Sound sound;

        Image image;   // TODO
        Sprite sprite; // TODO
        float scale;   // TODO
    } water;

} Cursor;

typedef struct {
    V2i position;
    Image image;

    struct {
        V2i position;
        Sprite sprite;
        Rectangle rect;
    } flame;
} Torch;

enum {
    L1_TORCH_LEFT,
    L1_TORCH_RIGHT,
    L1_TORCH_COUNT,
};

/* for now, the player animation type, and the player movement
 * state are the same thing since there's no physics */
typedef enum {
    PLAYER_ANIM_IDLE,
    PLAYER_ANIM_UP,
    PLAYER_ANIM_RIGHT,
    PLAYER_ANIM_DOWN,
    PLAYER_ANIM_LEFT,
    PLAYER_ANIM_COUNT,
} Player_Anim_Type;

#define PLAYER_SPEED 80

typedef struct {
    V2f position;
    V2i render_position;
    float scale;
    Image image;
    Sprite sprite;
    Rectangle rect;
    int foot_step;
} Player;

typedef struct {
    V2i position;
    int radius; // purely for debug visuals only position is used for collision

    /* once we collide with the way point,
     * we must transition to a new animation */
    Player_Anim_Type next_anim;
} Waypoint;

#define MAX_WAYPOINTS 10
typedef struct {
    Waypoint items[MAX_WAYPOINTS];
    int count;
    int target;
} Waypoints;

// for now these cant be saved, only printed out to the console
// i.e. debuggin purposes only
typedef struct {
    Waypoint *items;
    size_t count;
    size_t capacity;
} Editor_Waypoints;

typedef struct {
    Torch *items;
    size_t count;
    size_t capacity;
} Torches;

typedef struct {
    Torches torches;
    int torch_padding;
    int torch_scale;
    int torch_flame_scale;

    // TODO: remove
    V2i water_position;
    Rectangle water_rect;

    Rectangle barrier;
    bool barrier_burnt;

    Image image;
    int image_scale;
    Image door_image;

    Waypoints waypoints;
    Editor_Waypoints editor_waypoints;

    V2i player_start; // TODO: not using this yet
} Level;

typedef struct {
    Image image;
    int image_scale;
} Main_Menu;

static enum {
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    LEVEL_COUNT,
} curr_level = 0;

static enum {
    GAME_MODE_MAIN_MENU,
    GAME_MODE_GAME_ON,
    GAME_MODE_TRANSITION_LEVEL, // strech goal?
    GAME_MODE_GAME_OVER,
} game_mode = 0;

static int window_width = 800;
static int window_height = 640;

#define IDLE_ANIM_TIMEOUT 0.7f
#define WALK_ANIM_TIMEOUT 0.2f

void switch_player_anim(Sprite *sprite, Player_Anim_Type anim_type)
{
    sprite->animation.type = anim_type;
    sprite->animation.timeout = (anim_type == PLAYER_ANIM_IDLE) ? IDLE_ANIM_TIMEOUT : WALK_ANIM_TIMEOUT;
}

/* players coordinates are defined at the feet, but the sprite is drawn at the top left */
V2i get_player_render_position(V2f bottom_center, Sprite sprite, float scale)
{
    return v2i(bottom_center.x - scale*sprite.sub_image.size.x/2.0f,
               bottom_center.y - scale*sprite.sub_image.size.y);
}


void update_player_collision_rect(Player *player)
{
    V2i top_left = get_player_render_position(player->position, player->sprite, player->scale);
    player->rect.x = top_left.x;
    player->rect.y = top_left.y;
    player->rect.width  = player->sprite.sub_image.size.x*player->scale;
    player->rect.height = player->sprite.sub_image.size.y*player->scale;
}

void draw_cursor(Cursor cursor)
{
    draw_image_scaled(cursor.image, cursor.position.x, cursor.position.y, cursor.scale, cursor.scale);

    switch (cursor.element) {
    case ELEMENT_NONE:
    break;
    case ELEMENT_FIRE:
        draw_sprite(cursor.flame.sprite, cursor.position.x+cursor.image.width*cursor.scale/2, cursor.position.y);
    break;
    case ELEMENT_WATER:
        draw_sprite(cursor.water.sprite, cursor.position.x+cursor.image.width*cursor.scale/2, cursor.position.y);
    break;
    case ELEMENT_LIGHTNING:
    break;
    default:
    }
}

bool rectangle_contains_point(Rectangle r, int x, int y)
{
    return r.x <= x && x < r.x + r.width &&
           r.y <= y && y < r.y + r.height;
}

bool circle_contains_point(int cx, int cy, int radius, int x, int y)
{
    int dx = cx - x;
    int dy = cy - y;
    return dx*dx + dy*dy <= radius*radius;
}

Level load_level1(Sprite flame_sprite, Sprite water_sprite)
{
    Level level1 = {0};

    /* resourc allocation which we must free later */
    Image torch_image  = load_image("assets/torch.png");
    assert(torch_image.data);
    Image level1_image = load_image("assets/level1.png");
    assert(level1_image.data);
    Image level1_door_image = load_image("assets/level1_door.png");
    assert(level1_door_image.data);

    /* initialize torches */
    int flame_fudge_factor = 4;
    level1.torch_padding = 100;
    level1.torch_scale = 4;
    level1.torch_flame_scale = 3.0;
    Torch torch1 = {0};
    torch1.image = torch_image;
    torch1.position.x = level1.torch_padding;
    torch1.position.y = level1.torch_padding;
    torch1.flame.position.x = level1.torch_padding - flame_fudge_factor;
    torch1.flame.sprite = flame_sprite; // cursor is responsible for freeing this
    torch1.flame.sprite.scale = level1.torch_flame_scale;
    torch1.flame.position.y  = torch1.position.y;
    torch1.flame.position.y -= torch1.flame.sprite.image.height*level1.torch_flame_scale;
    torch1.flame.rect.x      = torch1.flame.position.x;
    torch1.flame.rect.y      = torch1.flame.position.y;
    torch1.flame.rect.width  = torch1.flame.sprite.sub_image.size.x*level1.torch_flame_scale;
    torch1.flame.rect.height = torch1.flame.sprite.sub_image.size.y*level1.torch_flame_scale;
    da_append(&level1.torches, torch1);

    Torch torch2 = {0};
    torch2.image = torch_image;
    torch2.position.x = window_width - level1.torch_padding - torch_image.width*level1.torch_scale;
    torch2.position.y = level1.torch_padding;
    torch2.flame.sprite = flame_sprite; // cursor is responsible for freeing this
    torch2.flame.sprite.scale = level1.torch_flame_scale;
    torch2.flame.position.x  = torch2.position.x - flame_fudge_factor;
    torch2.flame.position.y  = torch2.position.y;
    torch2.flame.position.y -= torch2.flame.sprite.image.height*level1.torch_flame_scale;
    torch2.flame.rect.x      = torch2.flame.position.x;
    torch2.flame.rect.y      = torch2.flame.position.y;
    torch2.flame.rect.width  = torch2.flame.sprite.sub_image.size.x*level1.torch_flame_scale;
    torch2.flame.rect.height = torch2.flame.sprite.sub_image.size.y*level1.torch_flame_scale;
    da_append(&level1.torches, torch2);

    /* water just a test */
    // level1.water_position = v2i(600, 450);
    // level1.water_rect.x = level1.water_position.x + water_sprite.sub_image.size.x/2;
    // level1.water_rect.y = level1.water_position.y + water_sprite.sub_image.size.y;
    // level1.water_rect.width  = water_sprite.sub_image.size.x*2.0;
    // level1.water_rect.height = water_sprite.sub_image.size.y*2.0;
    UNUSED(water_sprite);

    /* barrier which must be burnt */
    level1.barrier.x = 240;
    level1.barrier.y = 400;
    level1.barrier.width  = 20;
    level1.barrier.height = 100;

    /* image for the level (aka the map/room)*/
    level1.image = level1_image;
    level1.image_scale = 2;
    level1.door_image = level1_door_image;

    /* waypoints */
    level1.waypoints.items[0].radius = 5;
    level1.waypoints.items[0].position = v2i(window_width/2+40, level1.barrier.y + level1.barrier.height/2);
    level1.waypoints.items[0].next_anim = PLAYER_ANIM_UP;
    level1.waypoints.items[1].radius = 5;
    level1.waypoints.items[1].position = v2i(window_width/2, -40); // waypoint intentionally offscreen
    level1.waypoints.items[1].next_anim = PLAYER_ANIM_IDLE;
    level1.waypoints.count = 2;

    return level1;
}

void unload_level1(Level level1)
{
    unload_image(level1.torches.items[0].image);
    unload_image(level1.image);
    unload_image(level1.door_image);
}

void reset_level1(Level *level1)
{
    level1->barrier_burnt = false;
    level1->waypoints.target = 0;
}

void draw_level1(const Level *level1, const Player *player)
{
    /* level */
    draw_image_scaled(level1->image, 0, 0, level1->image_scale, level1->image_scale);

    /* torches */
    for (size_t i = 0; i < level1->torches.count; i++) {
        Torch torch = level1->torches.items[i];
        draw_image_scaled(torch.image, torch.position.x, torch.position.y, level1->torch_scale, level1->torch_scale);
        draw_sprite(torch.flame.sprite, torch.flame.position.x, torch.flame.position.y);
        if (DEBUG) draw_rectangle_lines(torch.flame.rect, YELLOW);
    }

    // draw_sprite(cursor.water.sprite, level1->water_position.x, level1->water_position.y);
    // if (DEBUG) draw_rectangle_lines(level1->water_rect, YELLOW);

    if (!level1->barrier_burnt) draw_rectangle(level1->barrier, BLUE);

    if (DEBUG) {
        // actual way points
        for (int i = 0; i < level1->waypoints.count; i++) {
            Waypoint wp = level1->waypoints.items[i];
            draw_circle(wp.position.x, wp.position.y, wp.radius, RED);
        }
        // editor debug way points
        for (size_t i = 0; i < level1->editor_waypoints.count; i++) {
            Waypoint wp = level1->editor_waypoints.items[i];
            draw_circle(wp.position.x, wp.position.y, wp.radius, BLUE);
        }
    }

    /* player */
    draw_sprite(player->sprite, player->render_position.x, player->render_position.y);
    if (DEBUG) draw_rectangle_lines(player->rect, YELLOW);

    /* draw door after player */
    V2i door_pos = v2i(window_width/2 - level1->image_scale*level1->door_image.width/2,0);
    draw_image_scaled(level1->door_image, door_pos.x, door_pos.y, level1->image_scale, level1->image_scale);
}

// Level load_level2(Sprite flame_sprite, Sprite water_sprite)
// {
//     Level level1 = {0};
//
//     /* resourc allocation which we must free later */
//     Image torch_image  = load_image("assets/torch.png");
//     assert(torch_image.data);
//     Image level1_image = load_image("assets/level1.png");
//     assert(level1_image.data);
//     Image level1_door_image = load_image("assets/level1_door.png");
//     assert(level1_door_image.data);
//
//     /* initialize torches */
//     int flame_fudge_factor = 4;
//     level1.torch_padding = 100;
//     level1.torch_scale = 4;
//     level1.torch_flame_scale = 3.0;
//     Torch torch1 = {0};
//     torch1.image = torch_image;
//     torch1.position.x = level1.torch_padding;
//     torch1.position.y = level1.torch_padding;
//     torch1.flame.position.x = level1.torch_padding - flame_fudge_factor;
//     torch1.flame.sprite = flame_sprite; // cursor is responsible for freeing this
//     torch1.flame.sprite.scale = level1.torch_flame_scale;
//     torch1.flame.position.y  = torch1.position.y;
//     torch1.flame.position.y -= torch1.flame.sprite.image.height*level1.torch_flame_scale;
//     torch1.flame.rect.x      = torch1.flame.position.x;
//     torch1.flame.rect.y      = torch1.flame.position.y;
//     torch1.flame.rect.width  = torch1.flame.sprite.sub_image.size.x*level1.torch_flame_scale;
//     torch1.flame.rect.height = torch1.flame.sprite.sub_image.size.y*level1.torch_flame_scale;
//     da_append(&level1.torches, torch1);
//
//     Torch torch2 = {0};
//     torch2.image = torch_image;
//     torch2.position.x = window_width - level1.torch_padding - torch_image.width*level1.torch_scale;
//     torch2.position.y = level1.torch_padding;
//     torch2.flame.sprite = flame_sprite; // cursor is responsible for freeing this
//     torch2.flame.sprite.scale = level1.torch_flame_scale;
//     torch2.flame.position.x  = torch2.position.x - flame_fudge_factor;
//     torch2.flame.position.y  = torch2.position.y;
//     torch2.flame.position.y -= torch2.flame.sprite.image.height*level1.torch_flame_scale;
//     torch2.flame.rect.x      = torch2.flame.position.x;
//     torch2.flame.rect.y      = torch2.flame.position.y;
//     torch2.flame.rect.width  = torch2.flame.sprite.sub_image.size.x*level1.torch_flame_scale;
//     torch2.flame.rect.height = torch2.flame.sprite.sub_image.size.y*level1.torch_flame_scale;
//     da_append(&level1.torches, torch2);
//
//     /* water just a test */
//     // level1.water_position = v2i(600, 450);
//     // level1.water_rect.x = level1.water_position.x + water_sprite.sub_image.size.x/2;
//     // level1.water_rect.y = level1.water_position.y + water_sprite.sub_image.size.y;
//     // level1.water_rect.width  = water_sprite.sub_image.size.x*2.0;
//     // level1.water_rect.height = water_sprite.sub_image.size.y*2.0;
//     UNUSED(water_sprite);
//
//     /* barrier which must be burnt */
//     level1.barrier.x = 240;
//     level1.barrier.y = 400;
//     level1.barrier.width  = 20;
//     level1.barrier.height = 100;
//
//     /* image for the level (aka the map/room)*/
//     level1.image = level1_image;
//     level1.image_scale = 2;
//     level1.door_image = level1_door_image;
//
//     /* waypoints */
//     level1.waypoints.items[0].radius = 5;
//     level1.waypoints.items[0].position = v2i(window_width/2+40, level1.barrier.y + level1.barrier.height/2);
//     level1.waypoints.items[0].next_anim = PLAYER_ANIM_UP;
//     level1.waypoints.items[1].radius = 5;
//     level1.waypoints.items[1].position = v2i(window_width/2, -40); // waypoint intentionally offscreen
//     level1.waypoints.items[1].next_anim = PLAYER_ANIM_IDLE;
//     level1.waypoints.count = 2;
//
//     return level1;
// }

int get_rendered_text_width(Font font, char* text, int text_length) // TODO: contribute
{
    int width = 0;
    for (int i = 0; i < text_length; ++i) {
        uint8_t ch = text[i];
        width += font.glyphs[ch-FIRST_CHAR].x_advance;
    }
    
    return width;
}

void draw_fps(Font font, String_Builder* sb)
{
    sb->count = 0; // reuse string builder memory
    sb_appendf(sb, "FPS:%d", get_avg_fps());
    int text_width = get_rendered_text_width(font, sb->items, sb->count);
    Rectangle fps_rect = {.width = text_width + 20, .height = font.height+5};
    draw_rectangle(fps_rect, WHITE);
    draw_text_at_base(font, sb->items, sb->count, 10, font.height, BLACK);
}

int main()
{
    disable_mouse_cursor();
    init_audio_device();
    init_window(window_width, window_height, "Creese 2D First Ever Jam!");

    Music bg_music = load_music_stream("assets/background_music.wav");
    Music win_music = load_music_stream("assets/Loop_Minstrel_Dance.wav");
    set_music_volume(win_music, 0.4);
    Sound game_start_sound = load_sound("assets/stone_door.wav");
    play_music_stream(bg_music);
    Sound secret_sound = load_sound("assets/secret_revealed.wav");
    Sound step_sound[2];
    step_sound[0] = load_sound("assets/step_0.wav");
    step_sound[1] = load_sound("assets/step_1.wav");
    set_sound_volume(step_sound[0], 0.3);
    set_sound_volume(step_sound[1], 0.3);

    /* initialize cursor */
    Cursor cursor = {0};
    cursor.image = load_image("assets/cursors/tile_0087.png");
    cursor.scale = 2.0;
    cursor.flame.image = load_image("assets/flames.png");
    cursor.flame.scale = 3.0;
    cursor.flame.sprite = load_sprite_from_image(cursor.flame.image, 7, 1, cursor.flame.scale);
    cursor.flame.sprite.animation.horizontal = true;
    cursor.flame.sound = load_sound("assets/flame.wav");
    cursor.water.sound = load_sound("assets/water.wav");
    cursor.water.scale = 3.0f;
    cursor.water.image = load_image("assets/water.png");
    cursor.water.sprite = load_sprite_from_image(cursor.water.image, 7, 1, cursor.water.scale);
    cursor.water.sprite.animation.horizontal = true;

    /* initialize level */
    Level level1 = load_level1(cursor.flame.sprite, cursor.water.sprite);

    Font debug_font = load_font("assets/RobotoMono-Medium.ttf", 32);
    Font title_font = load_font("assets/Metamorphous-Regular.ttf", 42);
    Font game_font = load_font("assets/Metamorphous-Regular.ttf", 32);
    String_Builder sb = {0};

    Main_Menu menu = {0};
    menu.image = load_image("assets/parchment-gold.png");
    menu.image_scale = 2.0;

    Player player = {0};
    player.image = load_image("assets/player.png");;
    assert(player.image.data);
    player.scale = 4.0f;
    player.sprite = load_sprite_from_image(player.image, 2, PLAYER_ANIM_COUNT, player.scale);
    player.sprite.animation.horizontal = true;
    player.sprite.animation.timeout = IDLE_ANIM_TIMEOUT;
    player.position.y = level1.barrier.y + level1.barrier.height;
    player.position.x = level1.barrier.x - player.scale*player.sprite.sub_image.size.x/2;

    while (!window_should_close()) {
        float dt = get_frame_time();

        switch (game_mode) {
        case GAME_MODE_MAIN_MENU:
            /* sound */
            update_music_stream(bg_music);

            /* update input */
            cursor.position = v2f2i(get_mouse_position());
            cursor.pressed  = is_mouse_button_pressed(MOUSE_BUTTON_LEFT);
            if (cursor.pressed) {
                game_mode = GAME_MODE_GAME_ON;
                play_sound(game_start_sound);
                continue;
            }

            /* reset to level 1 */
            curr_level = LEVEL_1;

            /* drawing */
            begin_drawing(WHITE);
                // background
                draw_image_scaled(menu.image, 0, 0, menu.image_scale, menu.image_scale);

                // title
                char* title_text = "Hurried Sponge Escape";
                int text_len = strlen(title_text);
                int rendered_text_width = get_rendered_text_width(title_font, title_text, text_len);
                draw_text_at_base(title_font, title_text, text_len, (window_width - rendered_text_width)/2, window_height/3, BLACK);

                // menu text
                char* text = "Click to play...";
                text_len = strlen(text);
                rendered_text_width = get_rendered_text_width(game_font, text, text_len);
                draw_text_at_base(game_font, text, text_len, (window_width - rendered_text_width)/2, 3*window_height/4, BLACK);

                draw_cursor(cursor);

                if (DEBUG) draw_fps(debug_font, &sb);
            end_drawing();
        break;
        case GAME_MODE_GAME_ON:
            /* sound */
            update_music_stream(bg_music);

            /* update input */
            cursor.position = v2f2i(get_mouse_position());
            cursor.pressed  = is_mouse_button_pressed(MOUSE_BUTTON_LEFT);

            /* debug */
            if (DEBUG && is_key_pressed(KEY_SPACE))
                player.sprite.animation.type = (player.sprite.animation.type + 1)%PLAYER_ANIM_COUNT;
            if (DEBUG && is_key_pressed(KEY_ENTER)) {
                stop_music_stream(bg_music);
                play_music_stream(win_music);
                game_mode = GAME_MODE_GAME_OVER;
            }
            if (DEBUG && is_mouse_button_pressed(MOUSE_BUTTON_RIGHT)) {
                Waypoint wp = {
                    .position = {
                        .x = cursor.position.x,
                        .y = cursor.position.y,
                    },
                    .radius = 10,
                };
                // TODO: depending on which level
                da_append(&level1.editor_waypoints, wp);
                printf("waypoint positions:\n");
                for (size_t i = 0; i < level1.editor_waypoints.count; i++) {
                    printf("    waypoint(%zu) = %d, %d\n", i, wp.position.x, wp.position.y);
                }
            }

            /* player physics (TODO: framerate independent movement) */
            switch (player.sprite.animation.type) {
            case PLAYER_ANIM_UP:    player.position.y -= dt*PLAYER_SPEED; break;
            case PLAYER_ANIM_RIGHT: player.position.x += dt*PLAYER_SPEED; break;
            case PLAYER_ANIM_DOWN:  player.position.y += dt*PLAYER_SPEED; break;
            case PLAYER_ANIM_LEFT:  player.position.x -= dt*PLAYER_SPEED; break;
            case PLAYER_ANIM_IDLE:                                        break;
            default:
            }
            player.render_position = get_player_render_position(player.position, player.sprite, player.scale);
            /* footsteps sound */
            if (player.sprite.animation.type != PLAYER_ANIM_IDLE) {
                if (player.sprite.animation.frame_just_changed) {
                    play_sound(step_sound[player.foot_step]);
                    player.foot_step = (player.foot_step + 1)%2;
                }
            }
            Waypoint target = level1.waypoints.items[level1.waypoints.target];
            update_player_collision_rect(&player);

            /* collision */
            switch (curr_level) {
            case LEVEL_1: {
                for (size_t i = 0; i < level1.torches.count; i++) {
                    Torch torch = level1.torches.items[i];
                    bool collision = rectangle_contains_point(torch.flame.rect, cursor.position.x, cursor.position.y);
                    if (cursor.pressed && collision) {
                        cursor.element = ELEMENT_FIRE;
                        play_sound(cursor.flame.sound);
                    }
                }
                bool barrier_collision = rectangle_contains_point(level1.barrier, cursor.position.x, cursor.position.y);
                if (cursor.pressed &&  barrier_collision && cursor.element == ELEMENT_FIRE) {
                    level1.barrier_burnt = true;
                    cursor.element = ELEMENT_NONE;
                    switch_player_anim(&player.sprite, PLAYER_ANIM_RIGHT);
                    play_sound(cursor.flame.sound);
                    play_sound(secret_sound);
                }
                if (rectangle_contains_point(player.rect, target.position.x, target.position.y)) {
                    switch_player_anim(&player.sprite, target.next_anim);
                    level1.waypoints.target++;
                    if (level1.waypoints.target >= level1.waypoints.count) {
                        curr_level = LEVEL_2;
                        level1.waypoints.target = 0;
                    }
                }
            } break;
            case LEVEL_2: {
                return 0;
            } break;
            case LEVEL_3:
            break;
            case LEVEL_4:
            break;
            default:
            }

            /* water collision */
            // if (cursor.pressed && rectangle_contains(level1.water_rect, cursor.position.x, cursor.position.y)) {
            //     cursor.element = ELEMENT_WATER;
            //     play_sound(cursor.water.sound);
            // }

            /* level specific animations */
            switch (curr_level) {
            case LEVEL_1: {
                for (size_t i = 0; i < level1.torches.count; i++)
                    update_animation(&level1.torches.items[i].flame.sprite, 7);
            } break;
            case LEVEL_2:
            break;
            case LEVEL_3:
            // update_animation(&cursor.water.sprite, 7);
            break;
            case LEVEL_4:
            break;
            default:
            }

            /* general animations */
            update_animation(&cursor.flame.sprite, 7);
            update_animation(&player.sprite, 2);

            /* drawing */
            begin_drawing(WHITE);
                switch (curr_level) {
                case LEVEL_1:
                    draw_level1(&level1, &player);
                break;
                case LEVEL_2: break;
                case LEVEL_3: break;
                case LEVEL_4: break;
                default:
                }

                /* cursor */
                draw_cursor(cursor);

                if (DEBUG) draw_fps(debug_font, &sb);
            end_drawing();
        break;
        case GAME_MODE_GAME_OVER:
            /* update input */
            cursor.position = v2f2i(get_mouse_position());
            
            if (is_key_pressed(KEY_ENTER)){
                game_mode = GAME_MODE_MAIN_MENU;
                stop_music_stream(win_music);
                play_music_stream(bg_music);
                continue;
            }

            // TODO: Add victory music
            update_music_stream(win_music);

            /* drawing */
            begin_drawing(WHITE);
                // background
                draw_image_scaled(menu.image, 0, 0, menu.image_scale, menu.image_scale); //TODO: Change the backgound

                // final message
                char* final_text = "You escaped...";
                int final_text_len = strlen(final_text);
                int rendered_final_text_width = get_rendered_text_width(game_font, final_text, final_text_len);
                draw_text_at_base(game_font, final_text, final_text_len, (window_width - rendered_final_text_width)/2, window_height/3, BLACK);

                // interact text
                char* interact_text = "Press ENTER to return to main menu...";
                int interact_text_len = strlen(interact_text);
                int rendered_interact_text_width = get_rendered_text_width(game_font, interact_text, interact_text_len);
                draw_text_at_base(game_font, interact_text, interact_text_len, (window_width - rendered_interact_text_width)/2, 3*window_height/4, BLACK);

                draw_cursor(cursor);

                if (DEBUG) draw_fps(debug_font, &sb);
            end_drawing();
        break;
        default:
        }
    }

    close_window();
    return 0;
}
