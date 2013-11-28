#include "gui.h" /* for the basic window */
#include "gui-dec.h" /* for the decoration utils */
#include <Windows.h>

/* A simple usage here */
int
main(int argc, char *argv[]) {
        gui_create(300, 300);
        gui_settrans(gui_hwnd(), 70); /* set 70% non-transperant */
        //gui_show(img->imageData, img->width, img->height, img->widthStep, 3);
        gui_wait(0);
        gui_destroy();
        return 0;
}
