#include "creese_2D.h"

#include "../nob.h"

/* Note this file doesn't do anything right now, but
 * just serves as an example for how we will add new C files.
 * This style of include-c-file-directly is called a unity build.
 * Any new c includes do need to update nob.c's game_dep_srcs[] */
#include "game_example_dependency.c"

#define DEBUG 1

// TODO: zelda unlock sound analogue
// TODO: water sound
// TODO: fire burn sound
// TODO: success music

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
    TORCH_LEFT,
    TORCH_RIGHT,
    TORCH_COUNT,
};

typedef struct {
    Torch torches[TORCH_COUNT];
    int torch_padding;
    int torch_scale;
    int torch_flame_scale;

    V2i water_position;
    Rectangle water_rect;

    Rectangle barrier;
    bool barrier_burnt;

    Image image;
    int image_scale;
} Level1;

typedef struct {
    Image image;
    int image_scale;
} Main_Menu;

static enum {
    GAME_MODE_MAIN_MENU,
    GAME_MODE_GAME_ON,
    GAME_MODE_TRANSITION_LEVEL, // strech goal?
    GAME_MODE_GAME_OVER,
} game_mode = 0;

typedef enum {
    PLAYER_ANIM_IDLE,
    PLAYER_ANIM_UP,
    PLAYER_ANIM_RIGHT,
    PLAYER_ANIM_DOWN,
    PLAYER_ANIM_LEFT,
    PLAYER_ANIM_COUNT,
} Player_Anim_Type;

typedef struct {
    V2f position;
    V2i render_position;
    float scale;
    Image image;
    Sprite sprite;
    Player_Anim_Type anim_type;
} Player;

static int window_width = 800;
static int window_height = 640;

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

bool rectangle_contains(Rectangle r, int x, int y)
{
    return r.x <= x && x < r.x + r.width &&
           r.y <= y && y < r.y + r.height;
}

V2i render_pos_from_bottom_center(V2f bottom_center, Sprite sprite, float scale)
{
    return v2i(bottom_center.x - scale*sprite.sub_image.size.x/2.0f,
               bottom_center.y - scale*sprite.sub_image.size.y);
}

Level1 load_level1(Sprite flame_sprite, Sprite water_sprite)
{
    Level1 level1 = {0};

    /* resourc allocation which we must free later */
    Image torch_image  = load_image("assets/torch.png");
    assert(torch_image.data);
    Image level1_image = load_image("assets/level1.png");
    assert(level1_image.data);

    /* initialize torches */
    int flame_fudge_factor = 4;
    level1.torch_padding = 100;
    level1.torch_scale = 4;
    level1.torch_flame_scale = 3.0;
    level1.torches[TORCH_LEFT].image = torch_image;
    level1.torches[TORCH_LEFT].position.x = level1.torch_padding;
    level1.torches[TORCH_LEFT].position.y = level1.torch_padding;
    level1.torches[TORCH_LEFT].flame.position.x = level1.torches[TORCH_LEFT].position.x - flame_fudge_factor;
    level1.torches[TORCH_LEFT].flame.sprite = flame_sprite; // cursor is responsible for freeing this
    level1.torches[TORCH_LEFT].flame.sprite.scale = level1.torch_flame_scale;
    level1.torches[TORCH_LEFT].flame.position.y  = level1.torches[TORCH_LEFT].position.y;
    level1.torches[TORCH_LEFT].flame.position.y -= level1.torches[TORCH_LEFT].flame.sprite.image.height*level1.torch_flame_scale;
    level1.torches[TORCH_LEFT].flame.rect.x = level1.torches[TORCH_LEFT].flame.position.x;
    level1.torches[TORCH_LEFT].flame.rect.y = level1.torches[TORCH_LEFT].flame.position.y;
    level1.torches[TORCH_LEFT].flame.rect.width  = level1.torches[TORCH_LEFT].flame.sprite.sub_image.size.x*level1.torch_flame_scale;
    level1.torches[TORCH_LEFT].flame.rect.height = level1.torches[TORCH_LEFT].flame.sprite.sub_image.size.y*level1.torch_flame_scale;

    level1.torches[TORCH_RIGHT].image = torch_image;
    level1.torches[TORCH_RIGHT].position.x = window_width - level1.torch_padding - level1.torches[TORCH_RIGHT].image.width*level1.torch_scale;
    level1.torches[TORCH_RIGHT].position.y = level1.torch_padding;
    level1.torches[TORCH_RIGHT].flame.sprite = flame_sprite; // cursor is responsible for freeing this
    level1.torches[TORCH_RIGHT].flame.sprite.scale = level1.torch_flame_scale;
    level1.torches[TORCH_RIGHT].flame.position.x = level1.torches[TORCH_RIGHT].position.x - flame_fudge_factor;
    level1.torches[TORCH_RIGHT].flame.position.y  = level1.torches[TORCH_RIGHT].position.y;
    level1.torches[TORCH_RIGHT].flame.position.y -= level1.torches[TORCH_RIGHT].flame.sprite.image.height*level1.torch_flame_scale;
    level1.torches[TORCH_RIGHT].flame.rect.x = level1.torches[TORCH_RIGHT].flame.position.x;
    level1.torches[TORCH_RIGHT].flame.rect.y = level1.torches[TORCH_RIGHT].flame.position.y;
    level1.torches[TORCH_RIGHT].flame.rect.width  = level1.torches[TORCH_RIGHT].flame.sprite.sub_image.size.x*level1.torch_flame_scale;
    level1.torches[TORCH_RIGHT].flame.rect.height = level1.torches[TORCH_RIGHT].flame.sprite.sub_image.size.y*level1.torch_flame_scale;

    /* water just a test */
    level1.water_position = v2i(600, 450);
    level1.water_rect.x = level1.water_position.x + water_sprite.sub_image.size.x/2;
    level1.water_rect.y = level1.water_position.y + water_sprite.sub_image.size.y;
    level1.water_rect.width  = water_sprite.sub_image.size.x*2.0;
    level1.water_rect.height = water_sprite.sub_image.size.y*2.0;

    /* barrier which must be burnt */
    level1.barrier.x = 240;
    level1.barrier.y = 400;
    level1.barrier.width  = 20;
    level1.barrier.height = 100;

    /* image for the level (aka the map/room)*/
    level1.image = level1_image;
    level1.image_scale = 2;

    return level1;
}

void unload_level1(Level1 level1)
{
    unload_image(level1.torches[TORCH_LEFT].image); // left and right images were shared
    unload_image(level1.image);
}

int get_rendered_text_width(Font font, char* text, int text_length)
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
    play_music_stream(bg_music);
    Sound game_start_sound = load_sound("assets/stone_door.wav");

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
    Level1 level1 = load_level1(cursor.flame.sprite, cursor.water.sprite);

    Font debug_font = load_font("assets/RobotoMono-Medium.ttf", 32);
    Font title_font = load_font("assets/Metamorphous-Regular.ttf", 42);
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
    player.sprite.animation.timeout = 0.7;
    player.position.y = level1.barrier.y + level1.barrier.height;
    player.position.x = level1.barrier.x - player.scale*player.sprite.sub_image.size.x/2;

    while (!window_should_close()) {
        switch (game_mode) {
        case GAME_MODE_MAIN_MENU:
            /* update input */
            cursor.position = v2f2i(get_mouse_position());
            cursor.pressed  = is_mouse_button_pressed(MOUSE_BUTTON_LEFT);
            if (cursor.pressed) {
                game_mode = GAME_MODE_GAME_ON;
                play_sound(game_start_sound);
                continue;
            }

            update_music_stream(bg_music);

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
                rendered_text_width = get_rendered_text_width(title_font, text, text_len);
                draw_text_at_base(title_font, text, text_len, (window_width - rendered_text_width)/2, 3*window_height/4, BLACK);

                draw_cursor(cursor);

                if (DEBUG) draw_fps(debug_font, &sb);
            end_drawing();
        break;
        case GAME_MODE_GAME_ON:
            /* update input */
            cursor.position = v2f2i(get_mouse_position());
            cursor.pressed  = is_mouse_button_pressed(MOUSE_BUTTON_LEFT);

            if (DEBUG && is_key_pressed(KEY_SPACE))
                player.sprite.animation.type = (player.sprite.animation.type + 1)%PLAYER_ANIM_COUNT;

            /* collision/state */
            for (int i = 0; i < TORCH_COUNT; i++) {
                if (cursor.pressed && rectangle_contains(level1.torches[i].flame.rect, cursor.position.x, cursor.position.y)) {
                    cursor.element = ELEMENT_FIRE;
                    play_sound(cursor.flame.sound);
                }
            }


            /* water collision */
            if (cursor.pressed && rectangle_contains(level1.water_rect, cursor.position.x, cursor.position.y)) {
                cursor.element = ELEMENT_WATER;
                play_sound(cursor.water.sound);
            }

            if (cursor.pressed && rectangle_contains(level1.barrier, cursor.position.x, cursor.position.y) &&
                cursor.element == ELEMENT_FIRE) {
                level1.barrier_burnt = true;
                cursor.element = ELEMENT_NONE;
                play_sound(cursor.flame.sound);
            }

            /* sound */
            update_music_stream(bg_music);

            /* update animations */
            update_animation(&cursor.flame.sprite, 7);
            update_animation(&cursor.water.sprite, 7);
            update_animation(&level1.torches[TORCH_LEFT].flame.sprite, 7);
            update_animation(&level1.torches[TORCH_RIGHT].flame.sprite, 7);
            update_animation(&player.sprite, 2);

            /* drawing */
            begin_drawing(WHITE);
                /* level */
                draw_image_scaled(level1.image, 0, 0, level1.image_scale, level1.image_scale);

                /* player */
                player.render_position = render_pos_from_bottom_center(player.position, player.sprite, player.scale);
                draw_sprite(player.sprite, player.render_position.x, player.render_position.y);

                /* torches */
                for (int i = 0; i < TORCH_COUNT; i++) {
                    draw_image_scaled(level1.torches[i].image, level1.torches[i].position.x, level1.torches[i].position.y, level1.torch_scale, level1.torch_scale);
                    draw_sprite(level1.torches[i].flame.sprite, level1.torches[i].flame.position.x, level1.torches[i].flame.position.y);

                    if (DEBUG)
                        draw_rectangle_lines(level1.torches[i].flame.rect, YELLOW);
                }
                draw_sprite(cursor.water.sprite, level1.water_position.x, level1.water_position.y);
                if (DEBUG) draw_rectangle_lines(level1.water_rect, YELLOW);

                draw_rectangle(level1.barrier, (level1.barrier_burnt) ? BLACK : BLUE);

                /* cursor */
                draw_cursor(cursor);

                if (DEBUG) draw_fps(debug_font, &sb);
            end_drawing();
        break;
        case GAME_MODE_GAME_OVER:
        break;
        default:
        }
    }

    close_window();
    return 0;
}
