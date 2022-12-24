#pragma once
#include "GWSM.h"

LRESULT CALLBACK KeyboardProc(_In_ int code, _In_ WPARAM wParam,
                              _In_ LPARAM lParam) {
    if (code < 0) {
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }

    if (wParam == VK_END) {
        GWSM::Instance().Terminate();
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
}
