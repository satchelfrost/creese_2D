#include "creese_2D.h"

#include "../nob.h"

/* Note this file doesn't do anything right now, but
 * just serves as an example for how we will add new C files.
 * This style of include-c-file-directly is called a unity build.
 * Any new c includes do need to update nob.c's game_dep_srcs[] */
#include "game_example_dependency.c"

#define DEBUG 1

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

    struct {
        Image image;
        Sprite sprite;
        float scale;
        Sound sound;
    } flame;
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

    Rectangle barrier;
    bool barrier_burnt;

    Image image;
    int image_scale;
} Level1;

typedef struct {
    V2f position;
    V2i render_position;
    float scale;
    Image image;
    Sprite sprite;
} Player;

static int window_width = 800;
static int window_height = 640;

void render_cursor(Cursor cursor)
{
    switch (cursor.element) {
    case ELEMENT_NONE:
    break;
    case ELEMENT_FIRE:
        draw_sprite(cursor.flame.sprite, cursor.position.x, cursor.position.y);
    break;
    case ELEMENT_WATER:
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

Level1 load_level1(Sprite flame_sprite)
{
    Level1 level1 = {0};

    /* resourc allocation which we must free later */
    Image torch_image  = load_image("assets/torch.png");
    assert(torch_image.data);
    Image level1_image = load_image("assets/level1.png");
    assert(level1_image.data);

    /* initialize torches */
    level1.torch_padding = 100;
    level1.torch_scale = 3;
    level1.torch_flame_scale = 2.0;
    level1.torches[TORCH_LEFT].image = torch_image;
    level1.torches[TORCH_LEFT].position.x = level1.torch_padding;
    level1.torches[TORCH_LEFT].position.y = level1.torch_padding;
    level1.torches[TORCH_LEFT].flame.position.x = level1.torches[TORCH_LEFT].position.x;
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
    level1.torches[TORCH_RIGHT].flame.position.x = level1.torches[TORCH_RIGHT].position.x;
    level1.torches[TORCH_RIGHT].flame.position.y  = level1.torches[TORCH_RIGHT].position.y;
    level1.torches[TORCH_RIGHT].flame.position.y -= level1.torches[TORCH_RIGHT].flame.sprite.image.height*level1.torch_flame_scale;
    level1.torches[TORCH_RIGHT].flame.rect.x = level1.torches[TORCH_RIGHT].flame.position.x;
    level1.torches[TORCH_RIGHT].flame.rect.y = level1.torches[TORCH_RIGHT].flame.position.y;
    level1.torches[TORCH_RIGHT].flame.rect.width  = level1.torches[TORCH_RIGHT].flame.sprite.sub_image.size.x*level1.torch_flame_scale;
    level1.torches[TORCH_RIGHT].flame.rect.height = level1.torches[TORCH_RIGHT].flame.sprite.sub_image.size.y*level1.torch_flame_scale;

    /* barrier which must be burnt */
    level1.barrier.x = 100;
    level1.barrier.y = 400;
    level1.barrier.width  = 50;
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

int main()
{
    // disable_mouse_cursor();
    init_audio_device();
    init_window(window_width, window_height, "Creese 2D First Ever Jam!");

    /* initialize assets */
    Cursor cursor = {0};
    cursor.flame.image = load_image("assets/flames.png");
    cursor.flame.scale = 3.0;
    cursor.flame.sprite = load_sprite_from_image(cursor.flame.image, 7, 1, cursor.flame.scale);
    cursor.flame.sprite.animation.horizontal = true;
    cursor.flame.sound = load_sound("assets/flame.wav");
    Font font = load_font("assets/RobotoMono-Medium.ttf", 32);
    String_Builder sb = {0};

    /* initialize level */
    Level1 level1 = load_level1(cursor.flame.sprite);

    Player player = {0};
    player.image = load_image("assets/player.png");;
    if (!player.image.data) return 1;
    player.scale = 4.0f;
    player.sprite = load_sprite_from_image(player.image, 2, 4, player.scale);
    player.sprite.animation.horizontal = true;
    player.position.y = window_height;
    player.position.x = window_width/2;

    while (!window_should_close()) {
        /* update input */
        cursor.position = v2f2i(get_mouse_position());
        cursor.pressed  = is_mouse_button_pressed(MOUSE_BUTTON_LEFT);

        if (is_key_pressed(KEY_SPACE))
            player.sprite.animation.type = (player.sprite.animation.type + 1)%4;

        /* physics/state */
        for (int i = 0; i < TORCH_COUNT; i++) {
            if (cursor.pressed && rectangle_contains(level1.torches[i].flame.rect, cursor.position.x, cursor.position.y)) {
                cursor.element = ELEMENT_FIRE;
                play_sound(cursor.flame.sound);
            }
        }

        if (cursor.pressed && rectangle_contains(level1.barrier, cursor.position.x, cursor.position.y) &&
            cursor.element == ELEMENT_FIRE) {
            level1.barrier_burnt = true;
            cursor.element = ELEMENT_NONE;
            play_sound(cursor.flame.sound);
        }

        /* update animations */
        update_animation(&cursor.flame.sprite, 7);
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
                draw_image_scaled(level1.torches[i].image, level1.torches[i].position.x, level1.torches[i].position.y, 3, 3);
                draw_sprite(level1.torches[i].flame.sprite, level1.torches[i].flame.position.x, level1.torches[i].flame.position.y);

                if (DEBUG)
                    draw_rectangle_lines(level1.torches[i].flame.rect, YELLOW);
            }
            draw_rectangle(level1.barrier, (level1.barrier_burnt) ? BLACK : BLUE);

            /* cursor */
            render_cursor(cursor);

            if (DEBUG) {
                Rectangle fps_rect = {.width = 125, .height = font.height+5};
                draw_rectangle(fps_rect, WHITE);
                sb.count = 0; // reuse string builder memory
                sb_appendf(&sb, "FPS:%d", get_avg_fps());
                draw_text_at_base(font, sb.items, sb.count, 10, font.height, BLACK);
            }
        end_drawing();
    }

    close_window();
    return 0;
}
