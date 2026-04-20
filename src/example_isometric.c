#include "creese_2D.h"

int main()
{
    int window_width = 768;
    int window_height = 384;
    init_window(window_width, window_height, "isometric");

    Image image = load_image("assets/stone.png");
    if (!image.data) return 1;

    int N = 3;
    int scale_down = 2;
    int tile_width = image.width/scale_down;
    int tile_height = image.height/scale_down;

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
    float det = 1.0f/(a*d - b*c);
    M2f screen_to_index = {
        ._11 =  d*det, ._12 = -b*det,
        ._21 = -c*det, ._22 =  a*det,
    };

    while (!window_should_close()) {
        /* use the mouse position and inverse to calculate tile index */
        Mouse mouse = get_mouse_position();
        V2i tile_idx = v2i2f(m2f_mul_vec(screen_to_index, v2f(mouse.x-window_width/2.0f, mouse.y)));
        bool in_bounds = 0 <= tile_idx.x && tile_idx.x < N &&
                         0 <= tile_idx.y && tile_idx.y < N;

        begin_drawing(BLUE);
            /* draw tiles */
            for (int y = 0; y < N; y++) {
                for (int x = 0; x < N; x++) {
                    V2f tile = m2f_mul_vec(index_to_screen, v2f(x, y));
                    tile.x += window_width/2.0f - tile_width/2.0f;
                    if (tile_idx.x == x && tile_idx.y == y && in_bounds)
                        draw_image_scaled_down_tint(image, tile.x, tile.y, scale_down, scale_down, RED);
                    else
                        draw_image_scaled_down(image, tile.x, tile.y, scale_down, scale_down);
                }
            }
        end_drawing();
    }

    close_window();
    return 0;
}
