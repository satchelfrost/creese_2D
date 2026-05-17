#include "creese_2D.h"

#include "../nob.h"

/* Note this file doesn't do anything right now, but
 * just serves as an example for how we will add new C files.
 * This style of include-c-file-directly is called a unity build.
 * Any new c includes do need to update nob.c's game_dep_srcs[] */
#include "game_example_dependency.c"

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
    } flame;
} Cursor;

typedef struct {
    Rectangle rect;
    Color color;
} Torch;

enum {
    TORCH_LEFT,
    TORCH_RIGHT,
    TORCH_COUNT,
};

typedef struct {
    Torch torches[TORCH_COUNT];
    int torch_padding;

    Rectangle barrier;
    bool barrier_burnt;

    Image image;
    int image_scale;
} Level1;

typedef struct {
    Image image;
    int image_scale;
} Main_Menu;

enum {
    MAIN_MENU,
    GAME,
    GAME_OVER,
};

typedef struct {
    V2f position;
    V2i render_position;
    float scale;
    Image image;
    Sprite sprite;
} Player;

void render_cursor(Cursor cursor)
{
    draw_image_scaled(cursor.image, cursor.position.x, cursor.position.y, cursor.scale, cursor.scale);

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

int get_rendered_text_width(Font font, char* text, int text_length) {
    int width = 0;
    for (int i = 0; i < text_length; ++i) {
        uint8_t ch = text[i];
        width += font.glyphs[ch-FIRST_CHAR].x_advance;
    }
    
    return width;
}

void draw_fps(Font font, String_Builder* sb) {
    sb->count = 0; // reuse string builder memory
    sb_appendf(sb, "FPS:%d", get_avg_fps());
    int text_width = get_rendered_text_width(font, sb->items, sb->count);
    Rectangle fps_rect = {.width = text_width + 20, .height = font.height+5};
    draw_rectangle(fps_rect, WHITE);
    draw_text_at_base(font, sb->items, sb->count, 10, font.height, BLACK);
}

int main()
{
    int window_width = 800;
    int window_height = 640;
    disable_mouse_cursor();
    init_window(window_width, window_height, "Creese 2D First Ever Jam!");

    int game_mode = MAIN_MENU;

    /* initialize assets */
    Cursor cursor = {0};
    cursor.flame.image = load_image("assets/flames.png");
    cursor.flame.scale = 5.0;
    cursor.flame.sprite = load_sprite_from_image(cursor.flame.image, 7, 1, cursor.flame.scale);
    cursor.flame.sprite.animation.horizontal = true;
    cursor.image = load_image("assets/cursors/tile_0087.png");
    cursor.scale = 2.0;

    Font font = load_font("assets/RobotoMono-Medium.ttf", 32);
    Font title_font = load_font("assets/Metamorphous-Regular.ttf", 42);
    String_Builder sb = {0};

    Main_Menu menu = {0};
    menu.image = load_image("assets/parchment-gold.png");
    menu.image_scale = 2.0;

    /* initialize level */
    Level1 level1 = {0};
    level1.torch_padding = 50;
    level1.torches[TORCH_LEFT].rect.width  = 50;
    level1.torches[TORCH_LEFT].rect.height = 100;
    level1.torches[TORCH_LEFT].rect.x = level1.torch_padding;
    level1.torches[TORCH_LEFT].rect.y = level1.torch_padding;
    level1.torches[TORCH_LEFT].color = RED;

    level1.torches[TORCH_RIGHT].rect.width  = 50;
    level1.torches[TORCH_RIGHT].rect.height = 100;
    level1.torches[TORCH_RIGHT].rect.x = window_width - level1.torch_padding - level1.torches[TORCH_RIGHT].rect.width;
    level1.torches[TORCH_RIGHT].rect.y = level1.torch_padding;
    level1.torches[TORCH_RIGHT].color = RED;

    level1.barrier.x = 100;
    level1.barrier.y = 400;
    level1.barrier.width  = 50;
    level1.barrier.height = 100;

    level1.image = load_image("assets/level1.png");
    level1.image_scale = 2;
    if (!level1.image.data) return 1;

    Player player = {0};
    player.image = load_image("assets/player.png");;
    if (!player.image.data) return 1;
    player.scale = 4.0f;
    player.sprite = load_sprite_from_image(player.image, 2, 4, player.scale);
    player.sprite.animation.horizontal = true;
    player.position.y = window_height;
    player.position.x = window_width/2;

    while (!window_should_close()) {
        switch (game_mode)
        {
        case MAIN_MENU:
            /* update input */
            if (is_key_pressed(KEY_ENTER)) {
                game_mode = GAME;
                continue;
            }
            
            /* physics/state */

            /* update animations */

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
                char* text = "Press ENTER to play...";
                text_len = strlen(text);
                rendered_text_width = get_rendered_text_width(font, text, text_len);
                draw_text_at_base(font, text, text_len, (window_width - rendered_text_width)/2, 3*window_height/4, BLACK);

                /* debug */
                draw_fps(font, &sb);
            end_drawing();
        break;
        case GAME:
            /* update input */
            cursor.position = v2f2i(get_mouse_position());
            cursor.pressed  = is_mouse_button_pressed(MOUSE_BUTTON_LEFT);

            if (is_key_pressed(KEY_SPACE))
                player.sprite.animation.type = (player.sprite.animation.type + 1)%4;

            /* physics/state */
            for (int i = 0; i < TORCH_COUNT; i++) {
                if (cursor.pressed && rectangle_contains(level1.torches[i].rect, cursor.position.x, cursor.position.y)) {
                    cursor.element = ELEMENT_FIRE;
                }
            }

            if (cursor.pressed && rectangle_contains(level1.barrier, cursor.position.x, cursor.position.y) &&
                cursor.element == ELEMENT_FIRE) {
                level1.barrier_burnt = true;
                cursor.element = ELEMENT_NONE;
            }

            /* update animations */
            update_animation(&cursor.flame.sprite, 7);
            update_animation(&player.sprite, 2);

            /* drawing */
            begin_drawing(WHITE);
                /* level */
                draw_image_scaled(level1.image, 0, 0, level1.image_scale, level1.image_scale);

                /* player */
                player.render_position = render_pos_from_bottom_center(player.position, player.sprite, player.scale);
                draw_sprite(player.sprite, player.render_position.x, player.render_position.y);

                /* cursor */
                render_cursor(cursor);
                for (int i = 0; i < TORCH_COUNT; i++)
                    draw_rectangle(level1.torches[i].rect, level1.torches[i].color);
                draw_rectangle(level1.barrier, (level1.barrier_burnt) ? BLACK : BLUE);

                /* debug */
                draw_fps(font, &sb);
            end_drawing();
        break;
        case GAME_OVER:
        break;
        default:
        }
    }

    close_window();
    return 0;
}
