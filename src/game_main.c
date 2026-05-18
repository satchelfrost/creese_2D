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
        Image image;
        Sprite sprite;
        float scale;
    } water;

} Cursor;

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
    V2i position;
    Sprite sprite;
    Rectangle rect;
} Torch_Flame;

typedef struct {
    Torch_Flame *items;
    size_t count;
    size_t capacity;
} Torch_Flames;

typedef struct {
    V2i position;
    Sprite sprite;
    Rectangle rect;
} Fountain_Water;

typedef struct {
    Fountain_Water *items;
    size_t count;
    size_t capacity;
} Fountain_Waters;

typedef struct {
    Torch_Flames torches;
    int torch_padding;
    int torch_scale;
    int torch_flame_scale;

    Fountain_Waters fountains;
    int foutain_water_scale;

    Image image;
    int image_scale;
    Image exit_door_image;
    Image entry_door_image;

    Waypoints waypoints;
    Editor_Waypoints editor_waypoints;

    V2f player_start;

    bool crate_burnt;
    Rectangle burn_crate_rect;
    Image burn_crate_image;
    V2i burn_crate_pos;
    float burn_crate_scale;

    bool fires_extinguished;
    Rectangle fires_rect;
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
    Image level1_image = load_image("assets/level1.png");
    assert(level1_image.data);
    Image level1_door_image = load_image("assets/level1_door.png");
    assert(level1_door_image.data);
    Image burn_crate_image = load_image("assets/crate01.png");
    assert(burn_crate_image.data);

    /* initialize torches */
    level1.torch_flame_scale = 2.0;
    Torch_Flame torch1 = {0};

    torch1.position     = v2i(273, 150);
    torch1.sprite       = flame_sprite; // cursor is responsible for freeing this
    torch1.sprite.scale = level1.torch_flame_scale;
    torch1.rect.x       = torch1.position.x - level1.torch_flame_scale*flame_sprite.sub_image.size.x/2;
    torch1.rect.y       = torch1.position.y - level1.torch_flame_scale*flame_sprite.sub_image.size.y/2;
    torch1.rect.width   = torch1.sprite.sub_image.size.x*level1.torch_flame_scale;
    torch1.rect.height  = torch1.sprite.sub_image.size.y*level1.torch_flame_scale;
    da_append(&level1.torches, torch1);

    Torch_Flame torch2 = {0};
    torch2.sprite       = flame_sprite; // cursor is responsible for freeing this
    torch2.sprite.scale = level1.torch_flame_scale;
    torch2.position     = v2i(529, 150);
    torch2.rect.x       = torch2.position.x - level1.torch_flame_scale*flame_sprite.sub_image.size.x/2;
    torch2.rect.y       = torch2.position.y - level1.torch_flame_scale*flame_sprite.sub_image.size.y/2;
    torch2.rect.width   = torch2.sprite.sub_image.size.x*level1.torch_flame_scale;
    torch2.rect.height  = torch2.sprite.sub_image.size.y*level1.torch_flame_scale;
    da_append(&level1.torches, torch2);

    /* water just a test */
    // level1.water_position = v2i(600, 450);
    // level1.water_rect.x = level1.water_position.x + water_sprite.sub_image.size.x/2;
    // level1.water_rect.y = level1.water_position.y + water_sprite.sub_image.size.y;
    // level1.water_rect.width  = water_sprite.sub_image.size.x*2.0;
    // level1.water_rect.height = water_sprite.sub_image.size.y*2.0;
    UNUSED(water_sprite);

    /* barrier which must be burnt */
    level1.burn_crate_rect.x = 280;
    level1.burn_crate_rect.y = 400;
    level1.burn_crate_rect.width  = 40;
    level1.burn_crate_rect.height = 100;
    level1.burn_crate_image = burn_crate_image;
    level1.burn_crate_scale = 3.0;
    level1.burn_crate_pos = v2i(302 - level1.burn_crate_scale*burn_crate_image.width/2,
                                446 - level1.burn_crate_scale*burn_crate_image.height/2);
    /* image for the level (aka the map/room)*/
    level1.image = level1_image;
    level1.image_scale = 2;
    level1.exit_door_image = level1_door_image;

    /* waypoints */
    level1.waypoints.items[0].radius = 5;
    level1.waypoints.items[0].position = v2i(434, 443);
    level1.waypoints.items[0].next_anim = PLAYER_ANIM_UP;
    level1.waypoints.items[1].radius = 5;
    level1.waypoints.items[1].position = v2i(window_width/2, -40); // waypoint intentionally offscreen
    level1.waypoints.items[1].next_anim = PLAYER_ANIM_IDLE;
    level1.waypoints.count = 2;

    level1.player_start = v2f(208, 468);

    return level1;
}

void unload_level1(Level level1)
{
    unload_image(level1.image);
    unload_image(level1.exit_door_image);
}

void reset_level1(Level *level1)
{
    level1->crate_burnt = false;
    level1->waypoints.target = 0;
}

void draw_level1(const Level *level1, const Player *player)
{
    /* level */
    draw_image_scaled(level1->image, 0, 0, level1->image_scale, level1->image_scale);

    /* torches */
    for (size_t i = 0; i < level1->torches.count; i++) {
        Torch_Flame torch = level1->torches.items[i];
        draw_sprite(torch.sprite, torch.rect.x, torch.rect.y);
        if (DEBUG) draw_rectangle_lines(torch.rect, YELLOW);
    }

    // draw_sprite(cursor.water.sprite, level1->water_position.x, level1->water_position.y);
    // if (DEBUG) draw_rectangle_lines(level1->water_rect, YELLOW);

    if (DEBUG && !level1->crate_burnt) draw_rectangle_lines(level1->burn_crate_rect, YELLOW);

    if (!level1->crate_burnt)
        draw_image_scaled(level1->burn_crate_image, level1->burn_crate_pos.x, level1->burn_crate_pos.y,
                          level1->burn_crate_scale, level1->burn_crate_scale);

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
    V2i door_pos = v2i(window_width/2 - level1->image_scale*level1->exit_door_image.width/2,0);
    draw_image_scaled(level1->exit_door_image, door_pos.x, door_pos.y, level1->image_scale, level1->image_scale);
}

Level load_level2(Sprite flame_sprite)
{
    Level level2 = {0};

    /* resourc allocation which we must free later */

    level2.image = load_image("assets/level2.png");
    assert(level2.image.data);
    printf("level 2 image loaded\n");
    level2.entry_door_image = load_image("assets/level2_entry_door.png");
    assert(level2.entry_door_image.data);
    level2.exit_door_image = load_image("assets/level2_exit_door.png");
    assert(level2.exit_door_image.data);

    level2.image_scale = 2;

    UNUSED(flame_sprite);

    return level2;
}

void draw_level2(const Level *level2, const Player *player)
{
    draw_image_scaled(level2->image, 0, 0, level2->image_scale, level2->image_scale);
    UNUSED(player);
}

void unload_level2(Level level2)
{
    unload_image(level2.image);
    unload_image(level2.exit_door_image);
    unload_image(level2.entry_door_image);
}

// void reset_level2(Level *level2)
// {
// }

Level load_level3(Sprite water_sprite, Sprite flame_sprite)
{
    Level level3 = {0};

    /* resourc allocation which we must free later */

    level3.image = load_image("assets/level3.png");
    assert(level3.image.data);
    printf("level 3 image loaded\n");
    level3.entry_door_image = load_image("assets/level3_entry_door.png");
    assert(level3.entry_door_image.data);
    level3.exit_door_image = load_image("assets/level3_exit_door.png");
    assert(level3.exit_door_image.data);

    level3.image_scale = 2;

    // water fountains
    level3.foutain_water_scale = 2.0;
    Fountain_Water fountain1 = {0};

    fountain1.position     = v2i(144, 165);
    fountain1.sprite       = water_sprite; // cursor is responsible for freeing this
    fountain1.sprite.scale = level3.foutain_water_scale;
    fountain1.rect.x       = fountain1.position.x - level3.foutain_water_scale*water_sprite.sub_image.size.x/2;
    fountain1.rect.y       = fountain1.position.y - level3.foutain_water_scale*water_sprite.sub_image.size.y/2;
    fountain1.rect.width   = fountain1.sprite.sub_image.size.x*level3.foutain_water_scale;
    fountain1.rect.height  = fountain1.sprite.sub_image.size.y*level3.foutain_water_scale;
    da_append(&level3.fountains, fountain1);

    Fountain_Water fountain2 = {0};
    fountain2.sprite       = water_sprite; // cursor is responsible for freeing this
    fountain2.sprite.scale = level3.foutain_water_scale;
    fountain2.position     = v2i(240, 485);
    fountain2.rect.x       = fountain2.position.x - level3.foutain_water_scale*water_sprite.sub_image.size.x/2;
    fountain2.rect.y       = fountain2.position.y - level3.foutain_water_scale*water_sprite.sub_image.size.y/2;
    fountain2.rect.width   = fountain2.sprite.sub_image.size.x*level3.foutain_water_scale;
    fountain2.rect.height  = fountain2.sprite.sub_image.size.y*level3.foutain_water_scale;
    da_append(&level3.fountains, fountain2);

    // fires
    level3.torch_flame_scale = 2.0;
    Torch_Flame torch1 = {0};

    torch1.position     = v2i(400, 355);
    torch1.sprite       = flame_sprite; // cursor is responsible for freeing this
    torch1.sprite.scale = level3.torch_flame_scale;
    torch1.rect.x       = torch1.position.x - level3.torch_flame_scale*flame_sprite.sub_image.size.x/2;
    torch1.rect.y       = torch1.position.y - level3.torch_flame_scale*flame_sprite.sub_image.size.y/2;
    torch1.rect.width   = torch1.sprite.sub_image.size.x*level3.torch_flame_scale;
    torch1.rect.height  = torch1.sprite.sub_image.size.y*level3.torch_flame_scale;
    da_append(&level3.torches, torch1);

    Torch_Flame torch2 = {0};
    torch2.sprite       = flame_sprite; // cursor is responsible for freeing this
    torch2.sprite.scale = level3.torch_flame_scale;
    torch2.position     = v2i(400, 315);
    torch2.rect.x       = torch2.position.x - level3.torch_flame_scale*flame_sprite.sub_image.size.x/2;
    torch2.rect.y       = torch2.position.y - level3.torch_flame_scale*flame_sprite.sub_image.size.y/2;
    torch2.rect.width   = torch2.sprite.sub_image.size.x*level3.torch_flame_scale;
    torch2.rect.height  = torch2.sprite.sub_image.size.y*level3.torch_flame_scale;
    da_append(&level3.torches, torch2);

    level3.fires_rect.x = torch2.position.x - level3.torch_flame_scale*flame_sprite.sub_image.size.x/2;
    level3.fires_rect.y = torch2.position.y - level3.torch_flame_scale*flame_sprite.sub_image.size.y/2;
    level3.fires_rect.width = torch2.sprite.sub_image.size.x*level3.torch_flame_scale;
    level3.fires_rect.height  = torch2.sprite.sub_image.size.y*level3.torch_flame_scale * 2;    

    // waypoints
    level3.waypoints.items[0].radius = 5;
    level3.waypoints.items[0].position = v2i(window_width + 40, 365); // waypoint intentionally offscreen
    level3.waypoints.items[0].next_anim = PLAYER_ANIM_IDLE;
    level3.waypoints.count = 1;

    // player start
    level3.player_start = v2f(110, 365);

    return level3;
}

void draw_level3(const Level *level3, const Player *player)
{
    draw_image_scaled(level3->image, 0, 0, level3->image_scale, level3->image_scale);

    // Fountains
    for (size_t i = 0; i < level3->fountains.count; i++) {
        Fountain_Water fountain = level3->fountains.items[i];
        draw_sprite(fountain.sprite, fountain.rect.x, fountain.rect.y);
        if (DEBUG) draw_rectangle_lines(fountain.rect, YELLOW);
    }

    /* player */
    draw_sprite(player->sprite, player->render_position.x, player->render_position.y);
    if (DEBUG) draw_rectangle_lines(player->rect, YELLOW);

    // fires
    if(!level3->fires_extinguished) 
    {
        for (size_t i = 0; i < level3->torches.count; i++) {
            Torch_Flame torch = level3->torches.items[i];
            draw_sprite(torch.sprite, torch.rect.x, torch.rect.y);
        }
        if (DEBUG) draw_rectangle_lines(level3->fires_rect, YELLOW);
    }

    /* draw door after player */
    V2i door_pos = v2i(window_width - level3->image_scale*level3->exit_door_image.width, 
                       (window_height/2 - level3->image_scale*level3->exit_door_image.height/2) + 16);
    draw_image_scaled(level3->exit_door_image, door_pos.x, door_pos.y, level3->image_scale, level3->image_scale);
}

void unload_level3(Level level3)
{
    unload_image(level3.image);
    unload_image(level3.exit_door_image);
    unload_image(level3.entry_door_image);
}

void reset_level3(Level *level3) 
{
    level3->fires_extinguished = false;
    level3->waypoints.target = 0;
}

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

void draw_cursor_pos(Font font, String_Builder* sb, V2f position)
{
    sb->count = 0; // reuse string builder memory
    sb_appendf(sb, "x, y = %.2f, %.2f", position.x, position.y);
    int text_width = get_rendered_text_width(font, sb->items, sb->count);
    Rectangle fps_rect = {.y = font.height, .width = text_width + 20, .height = font.height+5};
    draw_rectangle(fps_rect, WHITE);
    draw_text_at_base(font, sb->items, sb->count, 10, font.height*2, BLACK);
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
    Level level2 = load_level2(cursor.flame.sprite);
    Level level3 = load_level3(cursor.water.sprite, cursor.flame.sprite);

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
    player.position = level1.player_start;

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
                if (DEBUG) draw_cursor_pos(debug_font, &sb, cursor.position);
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
                printf("waypoint(%zu) = %d, %d\n", level1.editor_waypoints.count - 1, wp.position.x, wp.position.y);
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
            Waypoint target = {0};
            update_player_collision_rect(&player);
            
            /* collision */
            switch (curr_level) {
                case LEVEL_1: {
                target = level1.waypoints.items[level1.waypoints.target];
                for (size_t i = 0; i < level1.torches.count; i++) {
                    Torch_Flame torch = level1.torches.items[i];
                    bool collision = rectangle_contains_point(torch.rect, cursor.position.x, cursor.position.y);
                    if (cursor.pressed && collision) {
                        cursor.element = ELEMENT_FIRE;
                        play_sound(cursor.flame.sound);
                    }
                }
                bool crate_collision = rectangle_contains_point(level1.burn_crate_rect, cursor.position.x, cursor.position.y);
                if (cursor.pressed &&  crate_collision && cursor.element == ELEMENT_FIRE) {
                    level1.crate_burnt = true;
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
            } break;
            case LEVEL_3:
                target = level3.waypoints.items[level3.waypoints.target];

                /* water collision */
                for (size_t i = 0; i < level3.fountains.count; i++) {
                    Fountain_Water fount = level3.fountains.items[i];
                    bool collision = rectangle_contains_point(fount.rect, cursor.position.x, cursor.position.y);
                    if (cursor.pressed && collision) {
                        cursor.element = ELEMENT_WATER;
                        play_sound(cursor.water.sound);
                    }
                }
                bool fire_collision = rectangle_contains_point(level3.fires_rect, cursor.position.x, cursor.position.y);
                if (cursor.pressed && fire_collision && cursor.element == ELEMENT_WATER) {
                    level3.fires_extinguished = true;
                    cursor.element = ELEMENT_NONE;
                    switch_player_anim(&player.sprite, PLAYER_ANIM_RIGHT);
                    play_sound(cursor.water.sound);
                    play_sound(secret_sound);
                }

                if (rectangle_circle_collision(player.rect, target.position.x, target.position.y, target.radius)) {
                    switch_player_anim(&player.sprite, target.next_anim);
                    level3.waypoints.target++;
                    if (level3.waypoints.target >= level3.waypoints.count) {
                        curr_level = LEVEL_4;
                        level3.waypoints.target = 0;
                    }
                }
            break;
            case LEVEL_4:
            break;
            default:
            }


            /* level specific animations */
            switch (curr_level) {
            case LEVEL_1: {
                for (size_t i = 0; i < level1.torches.count; i++)
                    update_animation(&level1.torches.items[i].sprite, 7);
            } break;
            case LEVEL_2:
                // for (size_t i = 0; i < level2.torches.count; i++)
                //     update_animation(&level2.torches.items[i].sprite, 7);
            break;
            case LEVEL_3:
                for (size_t i = 0; i < level3.fountains.count; i++)
                    update_animation(&level3.fountains.items[i].sprite, 7);

                for (size_t i = 0; i < level3.torches.count; i++)
                    update_animation(&level3.torches.items[i].sprite, 7);
            break;
            case LEVEL_4:
            break;
            default:
            }

            /* general animations */
            update_animation(&cursor.flame.sprite, 7);
            update_animation(&cursor.water.sprite, 7);
            update_animation(&player.sprite, 2);

            /* drawing */
            begin_drawing(WHITE);
                switch (curr_level) {
                case LEVEL_1:
                    draw_level1(&level1, &player);
                break;
                case LEVEL_2:
                    draw_level2(&level2, &player);
                break;
                case LEVEL_3:
                    draw_level3(&level3, &player);
                break;
                case LEVEL_4:
                break;
                default:
                }

                /* cursor */
                draw_cursor(cursor);

                if (DEBUG) draw_fps(debug_font, &sb);
                if (DEBUG) draw_cursor_pos(debug_font, &sb, cursor.position);
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
