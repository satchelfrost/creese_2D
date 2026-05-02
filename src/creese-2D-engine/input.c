#define MAX_KEYBOARD_KEYS 512
#define MAX_KEY_PRESSED_QUEUE 16
#define MAX_CHAR_PRESSED_QUEUE 16
#define MAX_MOUSE_BUTTONS 8
#define MAX_GAMEPAD_BUTTONS 32
#define MAX_GAMEPAD_AXIS 8
    
static const unsigned short rgfw_to_creese_2D[] = {
    [KEY_NULL] =          RGFW_keyNULL,
    [KEY_APOSTROPHE] =    RGFW_apostrophe,
    [KEY_COMMA] =         RGFW_comma,
    [KEY_MINUS] =         RGFW_minus,
    [KEY_PERIOD] =        RGFW_period,
    [KEY_SLASH] =         RGFW_slash,
    [KEY_ESCAPE] =        RGFW_escape,
    [KEY_F1] =            RGFW_F1,
    [KEY_F2] =            RGFW_F2,
    [KEY_F3] =            RGFW_F3,
    [KEY_F4] =            RGFW_F4,
    [KEY_F5] =            RGFW_F5,
    [KEY_F6] =            RGFW_F6,
    [KEY_F7] =            RGFW_F7,
    [KEY_F8] =            RGFW_F8,
    [KEY_F9] =            RGFW_F9,
    [KEY_F10] =           RGFW_F10,
    [KEY_F11] =           RGFW_F11,
    [KEY_F12] =           RGFW_F12,
    [KEY_GRAVE] =         RGFW_backtick,
    [KEY_ZERO] =          RGFW_0,
    [KEY_ONE] =           RGFW_1,
    [KEY_TWO] =           RGFW_2,
    [KEY_THREE] =         RGFW_3,
    [KEY_FOUR] =          RGFW_4,
    [KEY_FIVE] =          RGFW_5,
    [KEY_SIX] =           RGFW_6,
    [KEY_SEVEN] =         RGFW_7,
    [KEY_EIGHT] =         RGFW_8,
    [KEY_NINE] =          RGFW_9,
    [KEY_EQUAL] =         RGFW_equals,
    [KEY_BACKSPACE] =     RGFW_backSpace,
    [KEY_TAB] =           RGFW_tab,
    [KEY_CAPS_LOCK] =     RGFW_capsLock,
    [KEY_LEFT_SHIFT] =    RGFW_shiftL,
    [KEY_LEFT_CONTROL] =  RGFW_controlL,
    [KEY_LEFT_ALT] =      RGFW_altL,
    [KEY_LEFT_SUPER] =    RGFW_superL,
     #ifndef RGFW_MACOS
    [KEY_RIGHT_SHIFT] =   RGFW_shiftR,
    [KEY_RIGHT_ALT] =     RGFW_altR,
     #endif
    [KEY_SPACE] =         RGFW_space,
    [KEY_A] =             RGFW_a,
    [KEY_B] =             RGFW_b,
    [KEY_C] =             RGFW_c,
    [KEY_D] =             RGFW_d,
    [KEY_E] =             RGFW_e,
    [KEY_F] =             RGFW_f,
    [KEY_G] =             RGFW_g,
    [KEY_H] =             RGFW_h,
    [KEY_I] =             RGFW_i,
    [KEY_J] =             RGFW_j,
    [KEY_K] =             RGFW_k,
    [KEY_L] =             RGFW_l,
    [KEY_M] =             RGFW_m,
    [KEY_N] =             RGFW_n,
    [KEY_O] =             RGFW_o,
    [KEY_P] =             RGFW_p,
    [KEY_Q] =             RGFW_q,
    [KEY_R] =             RGFW_r,
    [KEY_S] =             RGFW_s,
    [KEY_T] =             RGFW_t,
    [KEY_U] =             RGFW_u,
    [KEY_V] =             RGFW_v,
    [KEY_W] =             RGFW_w,
    [KEY_X] =             RGFW_x,
    [KEY_Y] =             RGFW_y,
    [KEY_Z] =             RGFW_z,
    [KEY_LEFT_BRACKET] =  RGFW_bracket,
    [KEY_BACKSLASH] =     RGFW_backSlash,
    [KEY_RIGHT_BRACKET] = RGFW_closeBracket,
    [KEY_SEMICOLON] =     RGFW_semicolon,
    [KEY_INSERT] =        RGFW_insert,
    [KEY_HOME] =          RGFW_home,
    [KEY_PAGE_UP] =       RGFW_pageUp,
    [KEY_DELETE] =        RGFW_delete,
    [KEY_END] =           RGFW_end,
    [KEY_PAGE_DOWN] =     RGFW_pageDown,
    [KEY_RIGHT] =         RGFW_right,
    [KEY_LEFT] =          RGFW_left,
    [KEY_DOWN] =          RGFW_down,
    [KEY_UP] =            RGFW_up,
    [KEY_NUM_LOCK] =      RGFW_numLock,
    [KEY_KP_DIVIDE] =     RGFW_kpSlash,
    [KEY_KP_MULTIPLY] =   RGFW_kpMultiply,
    [KEY_KP_SUBTRACT] =   RGFW_kpMinus,
    [KEY_KP_ENTER] =      RGFW_kpReturn,
    [KEY_KP_1] =          RGFW_kp1,
    [KEY_KP_2] =          RGFW_kp2,
    [KEY_KP_3] =          RGFW_kp3,
    [KEY_KP_4] =          RGFW_kp4,
    [KEY_KP_5] =          RGFW_kp5,
    [KEY_KP_6] =          RGFW_kp6,
    [KEY_KP_7] =          RGFW_kp7,
    [KEY_KP_8] =          RGFW_kp8,
    [KEY_KP_9] =          RGFW_kp9,
    [KEY_KP_0] =          RGFW_kp0,
    [KEY_KP_DECIMAL] =    RGFW_kpPeriod,
};

uint32_t convert_key_to_scancode(Keyboard_Key key)
{
    if (key > sizeof(rgfw_to_creese_2D)/sizeof(unsigned short)) return 0;
    return rgfw_to_creese_2D[key];
}

bool is_key_pressed(Keyboard_Key key)
{
    return RGFW_isKeyPressed(convert_key_to_scancode(key));
}

bool is_key_released(Keyboard_Key key)
{
    return RGFW_isKeyReleased(convert_key_to_scancode(key));
}

bool is_key_down(Keyboard_Key key)
{
    return RGFW_isKeyDown(convert_key_to_scancode(key));
}

V2i get_mouse_position()
{
    V2i mouse = {0};
    RGFW_window *window = get_window_ptr();
    RGFW_window_getMouse(window, &mouse.x, &mouse.y);
    return mouse;
}

bool mouse_inside_window()
{
    return RGFW_window_isMouseInside(get_window_ptr());
}

bool is_mouse_button_pressed(Mouse_Button button)
{
    return RGFW_isMousePressed(button);
}

bool is_mouse_button_released(Mouse_Button button)
{
    return RGFW_isMouseReleased(button);
}

bool is_mouse_button_down(Mouse_Button button)
{
    return RGFW_isMouseDown(button);
}

V2f get_mouse_wheel_move()
{
    float x, y;
    RGFW_getMouseScroll(&x, &y);
    return v2f(x, y);
}

V2f get_mouse_vector()
{
    float x, y;
    RGFW_getMouseVector(&x, &y);
    return v2f(x, y);
}

void poll_input_events()
{
    RGFW_pollEvents();
}
