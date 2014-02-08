/**
 * Lightweight singleton window for displaying image.
 *
 * @blackball
 */

#include "gui.h"
#include <Windows.h>
#include <string.h>
#include <assert.h>

typedef unsigned char uchar;

static struct {
        HINSTANCE instance;
        char *iname;
        
        HWND hwnd;
        HDC dc;
        HGDIOBJ bmp_obj;
        DWORD style;

        struct { int x,y,w,h; } rect;
} g_wp = {
        0,
        "x_x",
        
        0,
        0,
        0,
        WS_POPUP | WS_VISIBLE,

        {0, 0, 0, 0},
};

static __inline void _gui_cleanup(void); 
static __inline int  _gui_initsys(void); 
static __inline void _gui_resize(int w, int h);
static __inline void _gui_move(int x, int y);
static __inline int  _gui_setbmp(const uchar *data, int w, int h, int ws, int cn);
static __inline int  _gui_createbmpobj(int w, int h, int cn);
/* main window procedure */
static __inline LRESULT CALLBACK _gui_winproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
/* processing user keyboard events here */
static __inline void CALLBACK _gui_keyboard_cb(int event, int code);

int
gui_create(int w, int h) {
        if (g_wp.hwnd != 0) {
                _gui_resize(w, h);
                return 0;
        }
        
        if (_gui_initsys() != 0) {
                assert(0);
                return -1;
        }

        g_wp.hwnd = CreateWindow(g_wp.iname, "", g_wp.style, 0, 0, w, h, 0, 0, g_wp.instance, 0);
        g_wp.rect.x = 0;
        g_wp.rect.y = 0;
        g_wp.rect.w = w;
        g_wp.rect.h = h;

        if (g_wp.hwnd == 0) {
                assert(0);
                return -1;
        }
        
        g_wp.dc = CreateCompatibleDC(0);
        
        ShowWindow(g_wp.hwnd, SW_SHOW);
               
        return 0;
}

int
gui_show(const uchar *data, int w, int h, int ws, int cn) {
        if (cn != 3) {
                assert(0);
                return -1;
        }
        
        if (g_wp.hwnd != 0) {
                _gui_resize(w, h);
        }
        else {
                gui_create(w, h);
        }
        
        _gui_setbmp(data, w, h, ws, cn);
        InvalidateRect(g_wp.hwnd, 0, 0);
        return 0;
}

int
gui_wait(int delay) {
        int time0 = GetTickCount();
        int is_processed = 0;
        MSG msg;
        
        for (;;) {
                if( (delay > 0 && abs((int)(GetTickCount() - time0)) >= delay) || g_wp.hwnd == 0 ) {
                        return -1;
                }

                if (delay <= 0) {
                        GetMessage(&msg, 0, 0, 0);
                }
                else if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE) == FALSE) {
                        Sleep(1);
                        continue;
                }

                switch(msg.message) {
                case WM_DESTROY:
                case WM_CHAR:
                        DispatchMessage(&msg);
                        return (int)msg.wParam;

                case WM_KEYDOWN:
                        TranslateMessage(&msg);
                default:
                        DispatchMessage(&msg);
                        is_processed = 1;
                        break;
                }
                
                if (is_processed == 0) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                }
        }
                
        return 0;
}

int
gui_destroy() {
        DestroyWindow(g_wp.hwnd);
        g_wp.hwnd = 0;
        return 0;
}

void *
gui_hwnd(void) {
        return g_wp.hwnd;
}

static __inline void
_gui_cleanup(void) {
        if (g_wp.hwnd != 0) {
                DestroyWindow(g_wp.hwnd);
                g_wp.hwnd = 0;
        }
        
        if (g_wp.bmp_obj != 0) {
                DeleteObject(SelectObject(g_wp.dc, g_wp.bmp_obj));
                g_wp.bmp_obj = 0;
        }
        
        if (g_wp.dc != 0) {
                DeleteDC(g_wp.dc);
                g_wp.dc = 0;
        }
}

static __inline int
_gui_initsys() {
        static int need_init = 1;
        if (need_init) {
                WNDCLASS wndc;
                wndc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
                wndc.lpfnWndProc = _gui_winproc; // default window procedure
                wndc.cbClsExtra = 0;
                wndc.cbWndExtra = 0;
                wndc.hInstance = g_wp.instance;
                wndc.lpszClassName = g_wp.iname;
                wndc.lpszMenuName = 0;
                wndc.hIcon = LoadIcon(0, IDI_APPLICATION);
                wndc.hCursor = (HCURSOR)LoadCursor(0, (LPSTR)(size_t)IDC_CROSS);
                wndc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
                RegisterClass(&wndc);
                atexit(_gui_cleanup);
                need_init = 0;
        }
        return 0;
}

static __inline void
_gui_resize(int w, int h) {
        if (g_wp.rect.w != w || g_wp.rect.h != h) {
                SetWindowPos(g_wp.hwnd, 0, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
                g_wp.rect.w = w;
                g_wp.rect.h = h;
        }
}

static __inline void
_gui_move(int x, int y) {
        if (g_wp.rect.x != x || g_wp.rect.y != y) {
                MoveWindow(g_wp.hwnd, x, y, g_wp.rect.w, g_wp.rect.h, TRUE);
                g_wp.rect.x = x;
                g_wp.rect.y = y;
        }
}

/* Note the BITMAP is BGRBGR... and 4-byte aligned.
 */
static __inline int
_gui_setbmp(const uchar *data, int w, int h, int ws, int cn) {
        BITMAP bmp;
        HGDIOBJ ho;
        
        if (g_wp.bmp_obj == 0) {
                if (_gui_createbmpobj(w, h, cn) != 0) {
                        return -1;
                }
        }
        
        GdiFlush();
        ho = GetCurrentObject(g_wp.dc, OBJ_BITMAP);

        if (h == 0) {
                assert(0);
                return -1;
        }

        if (GetObject(ho, sizeof(bmp), &bmp) == 0) {
                assert(0);
                return -1;
        }

        if (bmp.bmWidth != w || bmp.bmHeight != h) {
                DeleteObject( SelectObject(g_wp.dc, g_wp.bmp_obj) );
                g_wp.bmp_obj = 0;
                _gui_createbmpobj(w, h, cn);

                ho = GetCurrentObject(g_wp.dc, OBJ_BITMAP);
                if (GetObject(ho, sizeof(bmp), &bmp) == 0) {
                        assert(0);
                        return -1;
                }
        }
        
        /* if the input image's alignment size == BITMAP's just one time copy */
        if (bmp.bmWidthBytes == ws) {
                memcpy(bmp.bmBits, data, sizeof(uchar) * ws * h);
        }
        else {
                int i = 0;
                for (; i < h; ++i) {
                        memcpy(bmp.bmBits, data + i * ws, sizeof(uchar) * w * cn);
                }
        }
        return 0;
}

static __inline int
_gui_createbmpobj(int w, int h, int cn) {
        uchar buffer[sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)];
        BITMAPINFO *binfo = (BITMAPINFO*)buffer;
        BITMAPINFOHEADER *bh = &(binfo->bmiHeader);
        void *pbits = 0;

        if (g_wp.dc == 0) {
                assert(0);
                return -1;
        }
        
        memset(bh, 0, sizeof(*bh));
        bh->biSize = sizeof(BITMAPINFOHEADER);
        bh->biWidth = w;
        bh->biHeight = h;
        bh->biPlanes = 1;
        bh->biBitCount = (unsigned short)(8 * cn);
        bh->biCompression = BI_RGB;

        g_wp.bmp_obj = SelectObject(g_wp.dc, CreateDIBSection(g_wp.dc, binfo, DIB_RGB_COLORS, &pbits, 0, 0));
        if (g_wp.bmp_obj == NULL) {
                assert(0);
                return -1;
        }

        return 0;
}

static __inline void CALLBACK 
_gui_keyboard_cb(int event, int code) {
        switch(event) {
        case WM_KEYDOWN:
                switch(code) {
                case VK_LEFT:
                        _gui_move(g_wp.rect.x - 10, g_wp.rect.y);
                        break;
                case VK_RIGHT:
                        _gui_move(g_wp.rect.x + 10, g_wp.rect.y);
                        break;
                case VK_UP:
                        _gui_move(g_wp.rect.x, g_wp.rect.y - 10);
                        break;
                case VK_DOWN:
                        _gui_move(g_wp.rect.x, g_wp.rect.y + 10);
                        break;
                }
                break;
        case WM_CHAR:
                switch(code) {
                case VK_ESCAPE:
                        DestroyWindow(g_wp.hwnd);
                        g_wp.hwnd = 0;
                        break;
                }
        }
}

static __inline LRESULT CALLBACK
_gui_winproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        UINT uhit; 
        
        switch(msg) {
        case WM_NCHITTEST:
                uhit = DefWindowProc(hwnd, WM_NCHITTEST, wparam, lparam);
                if (uhit == HTCLIENT) {
                        // cheat -_-
                        return HTCAPTION;
                }
                else {
                        return uhit;
                }
                break;
                
        case WM_PAINT:
                if (g_wp.bmp_obj != 0) {
                        PAINTSTRUCT paint;
                        HDC pdc = BeginPaint(hwnd, &paint);
                        BitBlt(pdc, 0, 0, g_wp.rect.w, g_wp.rect.h, g_wp.dc, 0, 0, SRCCOPY );
                        EndPaint(hwnd, &paint);
                }
                break;
                
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
                _gui_keyboard_cb(msg, wparam);
                break;
                
        default:
                return DefWindowProc(hwnd, msg, wparam, lparam);

        }
        return 0;
}
