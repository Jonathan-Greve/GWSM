#include "pch.h"

#include "GuildWarsSM.h"
#include "NewWndProc.h"

LRESULT CALLBACK SafeWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) noexcept
{
    __try
    {
        return NewWndProc(hWnd, Message, wParam, lParam);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return CallWindowProc(reinterpret_cast<WNDPROC>(DefaultWndProc), hWnd, Message, wParam, lParam);
    }
}
