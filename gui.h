#ifndef BB_GUI_H
#define BB_GUI_H

int gui_create(int w, int h);
int gui_show(const unsigned char *imgData, int w, int h, int ws, int cn);
int gui_wait(int delay);
int gui_destroy(void);

void *gui_hwnd(void); /* used for extension */

#endif 