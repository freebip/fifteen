#include "libbip.h"
#include "fifteen.h"

struct regmenu_ menu_screen = { 55, 1, 0, dispatch_screen, keypress_screen, 0, 0, show_screen, 0, 0 };
struct appdata_t** appdata_p;
struct appdata_t* appdata;

int main(int p, char** a)
{
    show_screen((void*)p);
}

unsigned short randint(short max)
{
    appdata->randseed = appdata->randseed * get_tick_count();
    appdata->randseed++;
    return ((appdata->randseed >> 16) * max) >> 16;
}

void init_board()
{
    for (int i = 0; i < 16; i++)
        appdata->board[i] = i;

    for (int i = 15; i >= 1; i--)
    {
        int j = randint(37) % (i + 1);
        byte tmp = appdata->board[j];
        appdata->board[j] = appdata->board[i];
        appdata->board[i] = tmp;
    }
    appdata->is_win = 0;
}


void keypress_screen()
{
    show_menu_animate(appdata->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);
};

int get_symbols_width(byte value, int spacing)
{
    struct res_params_ res_params;

    int max = 100;
    int x = 0;

    while (max)
    {
        if (value / max)
            break;
        max = max / 10;
    }

    do
    {
        int mm = max == 0 ? 0 : value / max;

        get_res_params(ELF_INDEX_SELF, mm, &res_params);
        x += res_params.width + spacing;
        if (max == 0)
            break;

        value = value % max;
        max = max / 10;
    } while (max);

    return x > 0 ? x - spacing : 0;
}

void print_digits(int value, int x, int y, int spacing)
{
    struct res_params_ res_params;

    int max = 100000;

    while (max)
    {
        if (value / max)
            break;
        max = max / 10;
    }

    do
    {
        int mm = max == 0 ? 0 : value / max;

        get_res_params(ELF_INDEX_SELF, mm, &res_params);
        show_elf_res_by_id(0, mm, x, y);
        x += res_params.width + spacing;
        if (max == 0)
            break;

        value = value % max;
        max = max / 10;
    } while (max);
}

void show_screen(void* p)
{
    appdata_p = (struct appdata_t**)get_ptr_temp_buf_2();

    if ((p == *appdata_p) && get_var_menu_overlay()) {
        appdata = *appdata_p;
        *appdata_p = (struct appdata_t*)NULL;
        reg_menu(&menu_screen, 0);
        *appdata_p = appdata;
    }
    else {
        reg_menu(&menu_screen, 0);
        *appdata_p = (struct appdata_t*)pvPortMalloc(sizeof(struct appdata_t));
        appdata = *appdata_p;
        _memclr(appdata, sizeof(struct appdata_t));
        appdata->proc = (Elf_proc_*)p;
        appdata->randseed = get_tick_count();
    }

    if (p && appdata->proc->elf_finish)
        appdata->ret_f = appdata->proc->elf_finish;
    else
        appdata->ret_f = show_watchface;

    init_board();

    draw_screen();

    // не выключаем экран, не выключаем подсветку
    set_display_state_value(8, 1);
    set_display_state_value(4, 1);
    set_display_state_value(2, 0);
}

int dispatch_screen(void* p)
{
    struct gesture_* gest = (struct gesture_*)p;

    if (gest->gesture != GESTURE_CLICK)
        return 0;

    if (appdata->is_win)
    {
        init_board();
    }
    else 
    {
        int x = gest->touch_pos_x / 44;
        int y = gest->touch_pos_y / 44;

        if (appdata->board[4 * y + x])
        {
            int index = -1;
            if (x > 0 && !appdata->board[4 * y + x - 1])
                index = 4 * y + x - 1;
            else if (x < 4 && !appdata->board[4 * y + x + 1])
                index = 4 * y + x + 1;
            else if (y > 0 && !appdata->board[4 * (y - 1) + x])
                index = 4 * (y - 1) + x;
            else if (y < 4 && !appdata->board[4 * (y + 1) + x])
                index = 4 * (y + 1) + x;

            if (index != -1)
            {
                appdata->board[index] = appdata->board[4 * y + x];
                appdata->board[4 * y + x] = 0;
            }
        
            // check win

            appdata->is_win = 1;
            for(int i=0; i<15;i++)
                if (appdata->board[i] != i + 1)
                {
                    appdata->is_win = 0;
                    break;
                }
        }
    }

    draw_screen();
    repaint_screen_lines(1, 176);
    return 0;
}

void draw_screen()
{
    set_graph_callback_to_ram_1();
    set_bg_color(COLOR_BLACK);
    fill_screen_bg();

    for (int i = 0; i < 16; i++)
    {
        int x = i % 4;
        int y = i / 4;

        int x44_1 = x * 44 + 1;
        int y44_1 = y * 44 + 1;

        if (appdata->board[i])
        {
            set_fg_color(appdata->is_win? COLOR_YELLOW : COLOR_BLUE);
            draw_filled_rect(x44_1, y44_1, x44_1 + 42, y44_1 + 42);

            draw_filled_rect_bg(x44_1, y44_1, x44_1 + 1, y44_1 + 1);
            draw_filled_rect_bg(x44_1 + 41, y44_1 + 41, x44_1 + 42, y44_1 + 42);
            draw_filled_rect_bg(x44_1, y44_1 + 41, x44_1 + 1, y44_1 + 42);
            draw_filled_rect_bg(x44_1 + 41, y44_1, x44_1 + 42, y44_1 + 1);


            int width = get_symbols_width(appdata->board[i], 2);
            print_digits(appdata->board[i], 22 - width / 2 + x * 44, 11+ y44_1, 2);
        }
        else 
        {
            set_fg_color(COLOR_GREEN);
            if (i > 3)
                draw_horizontal_line(y * 44 - 1, 1 + x44_1, x44_1 + 40);
            if (i < 12)
                draw_horizontal_line(y * 44 + 44, 1 + x44_1, x44_1 + 40);
            if (i % 4)
                draw_vertical_line(x * 44 - 1, 1 + y44_1, y44_1 + 40);
            if ((i-3) % 4)
                draw_vertical_line(x * 44 + 44, 1 + y44_1, y44_1 + 40);
        }
    }
}