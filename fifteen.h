#ifndef __FIFTEEN_H__
#define __FIFTEEN_H__

#include "libbip.h"

typedef unsigned short word;
typedef unsigned char byte;

struct appdata_t
{
    Elf_proc_* proc;
    void* ret_f;
    byte board[16];
    int randseed;
    int is_win;
};

void show_screen(void* return_screen);
void keypress_screen();
int dispatch_screen(void* p);
void draw_screen();

#endif