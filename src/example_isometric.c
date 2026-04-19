#include "creese_2D.h"

int main()
{
    int window_width = 768;
    int window_height = 384;
    init_window(window_width, window_height, "isometric");

    Image image = load_image("assets/stone.png");
    if (!image.data) return 1;

    int N = 3;
    int scale = 2;
    int tile_width = image.width/scale;
    int tile_height = image.height/scale;
    V2f i_hat = v2f(1.0*tile_width*0.5, 0.5*tile_height);
    V2f j_hat = v2f(-1.0*tile_width*0.5, 0.5*tile_height);
    float a = i_hat.x;
    float b = j_hat.x;
    float c = i_hat.y;
    float d = j_hat.y;
    float det = 1.0f/(a*d - b*c);
    V2f inv_i = v2f(d*det, -c*det);
    V2f inv_j = v2f(-b*det, a*det);

    Color cursor_color = BLACK;
    cursor_color.a /= 2;

    while (!window_should_close()) {
        Mouse mouse = get_mouse_position();
        V2f inv_x = v2f_mul(inv_i, v2f(mouse.x-window_width/2.0f, mouse.x-window_width/2.0f));
        V2f inv_y = v2f_mul(inv_j, v2f(mouse.y, mouse.y));
        V2i coords_by_mouse = v2i2f(v2f_add(inv_x, inv_y));

        begin_drawing(BLUE);
            /* draw tiles */
            for (int y = 0; y < N; y++) {
                for (int x = 0; x < N; x++) {
                    V2f xv = v2f_mul(i_hat, v2f(x, x));
                    V2f yv = v2f_mul(j_hat, v2f(y, y));
                    V2f coord = v2f_add(xv, yv);
                    if (coords_by_mouse.x == x && coords_by_mouse.y == y &&
                        0 <= coords_by_mouse.x && coords_by_mouse.x < N &&
                        0 <= coords_by_mouse.y && coords_by_mouse.y < N)
                        draw_image_scaled_down_tint(image, coord.x + window_width/2.0f - tile_width/2.0f, coord.y, scale, scale, RED);
                    else
                        draw_image_scaled_down(image, coord.x + window_width/2.0f - tile_width/2.0f, coord.y, scale, scale);
                }
            }

            draw_circle(mouse.x, mouse.y, 20, cursor_color);
        end_drawing();
    }

    close_window();
    return 0;
}
