#include "creese_2D.h"

int main()
{
    int window_width = 500;
    int window_height = 500;
    init_window(window_width, window_height, "isometric");

    /* we have two separate sprite images to avoid some overdraw
     * 5ms improvement tested on 10 year old laptop */
    Image front_cube = load_image("assets/iso_cube.png");   // only rendered for front cubes
    Image top_cube = load_image("assets/iso_cube_top.png"); // only rendered for top cubes (and not selected)
    if (!top_cube.data) return 1;
    if (!front_cube.data) return 1;

    int N = 5;
    int scale_up = 4;
    int tile_width = top_cube.width*scale_up;
    int tile_height = top_cube.height*scale_up;

    /* good explanation of the math for this: https://youtu.be/04oQ2jOUjkU?si=Rr0un32qptYn9XLL */
    // equation for tile index (x, y) to screen coordinates:
    // | i.x     j.x | | x |
    // |             | |   | = x*i + y*j
    // | i.y     j.y | | y |
    M2f index_to_screen = {
        ._11 =  tile_width*0.5, ._12 =  -tile_width*0.5,
        ._21 = tile_height*0.5, ._22 =  tile_height*0.5,
    };

    /* calculate the inverse so that we can use mouse coordinates to find the tile index */
    //            -1
    // | a     b |       1     | d      -b |
    // |         | = --------- |           |
    // | c     d |   (ad - bc) | -c      a |
    float a = index_to_screen.c[0], b = index_to_screen.c[1];
    float c = index_to_screen.c[2], d = index_to_screen.c[3];
    float inv_det = 1.0f/(a*d - b*c);
    M2f screen_to_index = {
        ._11 =  d*inv_det, ._12 = -b*inv_det,
        ._21 = -c*inv_det, ._22 =  a*inv_det,
    };

    while (!window_should_close()) {
        /* use the mouse position and inverse to calculate tile index */
        V2i mouse = get_mouse_position();
        V2i tile_idx = v2i2f(m2f_mul_vec(screen_to_index, v2f(mouse.x-window_width/2.0f, mouse.y)));
        bool in_bounds = 0 <= tile_idx.x && tile_idx.x < N &&
                         0 <= tile_idx.y && tile_idx.y < N;

        if (is_key_pressed(KEY_SPACE)) log_fps();
        begin_drawing(BLUE);
            /* draw tiles */
            for (int y = 0; y < N; y++) {
                for (int x = 0; x < N; x++) {
                    V2f tile = m2f_mul_vec(index_to_screen, v2f(x, y));
                    tile.x += window_width/2.0f - tile_width/2.0f;
                    bool highlighted = tile_idx.x == x && tile_idx.y == y && in_bounds;
                    if (highlighted) {
                        tile.y -= tile_height/2.0f;
                        draw_image_scaled_tint(front_cube, tile.x, tile.y, scale_up, scale_up, RED);
                    } else {
                        Image img = (x == N - 1 || y == N - 1) ? front_cube : top_cube;
                        draw_image_scaled(img, tile.x, tile.y, scale_up, scale_up);
                    }
                }
            }
        end_drawing();
    }

    close_window();
    return 0;
}
