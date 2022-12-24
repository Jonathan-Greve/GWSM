#pragma once
#include "GuildWarsSM.h"

LRESULT CALLBACK KeyboardProc(_In_ int code, _In_ WPARAM wParam,
                              _In_ LPARAM lParam) {
    if (code < 0) {
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }

    if (wParam == VK_END) {
        GuildWarsSM::Instance().Terminate();
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
}
