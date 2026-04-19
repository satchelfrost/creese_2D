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
    V2f i_hat = v2f(1.0*tile_width*0.5, 0.5*tile_height);
    V2f j_hat = v2f(-1.0*tile_width*0.5, 0.5*tile_height);

    /* calculate the inverse so that we can use mouse coordinates to find the tile index */
    //            -1
    // | a     b |       1     | d      -c |
    // |         | = --------- |           |
    // | c     d |   (ad - bc) | -b      a |
    float a = i_hat.x, b = j_hat.x;
    float c = i_hat.y, d = j_hat.y;
    float det = 1.0f/(a*d - b*c);
    V2f inv_i = v2f(d*det, -c*det);
    V2f inv_j = v2f(-b*det, a*det);

    while (!window_should_close()) {
        /* use the mouse position and inverse to calculate tile index */
        Mouse mouse = get_mouse_position();
        V2f inv_vx = v2f_mul(inv_i, v2f(mouse.x-window_width/2.0f, mouse.x-window_width/2.0f));
        V2f inv_vy = v2f_mul(inv_j, v2f(mouse.y, mouse.y));
        V2i tile_idx = v2i2f(v2f_add(inv_vx, inv_vy));
        bool in_bounds = 0 <= tile_idx.x && tile_idx.x < N &&
                         0 <= tile_idx.y && tile_idx.y < N;

        begin_drawing(BLUE);
            /* draw tiles */
            for (int y = 0; y < N; y++) {
                for (int x = 0; x < N; x++) {
                    V2f vx = v2f_mul(i_hat, v2f(x, x));
                    V2f vy = v2f_mul(j_hat, v2f(y, y));
                    V2f tile = v2f_add(vx, vy);
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
