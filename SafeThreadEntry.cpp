#include "pch.h"
#include "SafeThreadEntry.h"

#include "ThreadEntry.h"

DWORD __stdcall SafeThreadEntry(LPVOID module) {
    dll_module = static_cast<HMODULE>(module);
    __try {
        ThreadEntry(nullptr);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
