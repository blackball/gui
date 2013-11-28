/**
 * Decoration functions here.
 *
 * The set/del transparent codes are adopted from http://www.catch22.net/
 *
 * @blackball
 */

#include "gui-dec.h"
#include <windows.h>
#include <tchar.h>

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED 0x00080000
#endif

typedef BOOL (WINAPI * SLWAProc)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

#define LWA_COLORKEY 0x00000001
#define LWA_ALPHA    0x00000002

#define ULW_COLORKEY 0x00000001
#define ULW_ALPHA    0x00000002
#define ULW_OPAQUE   0x00000004

int 
gui_settrans(HWND hwnd, int percent) {
        SLWAProc pSLWA;
        HMODULE  hUser32;
        hUser32 = GetModuleHandle(_T("USER32.DLL"));
        pSLWA = (SLWAProc)GetProcAddress(hUser32, (char *)"SetLayeredWindowAttributes");

        if(pSLWA == 0) {
                return FALSE;
        }

        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        return pSLWA(hwnd, 0, (BYTE)((255 * percent) / 100), LWA_ALPHA);
}

int 
gui_deltrans(HWND hwnd){   
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
        RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
        return TRUE;
}

