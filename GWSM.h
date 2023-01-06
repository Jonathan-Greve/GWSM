#pragma once
#include "Utilities/Hook.h"
#include "ClientDataUpdater.h"

// Forwards declarations. The header files will be included in the .cpp file
// This speeds up compilation.
namespace GW
{
struct HookStatus;
}

// Set to the default WindowProc in Init. This is the WindowProc the GW client creates when launched.
// We store a copy so we can later restore current_GW_window_handle to the default WindowProc.
inline long DefaultWndProc = 0;
inline HWND current_GW_window_handle;

// Handle to keyboard hook. We use this when intercepting 'end' key press to terminate our dll.
inline HHOOK keyboard_hook_handle;

// Store the hook for the update method which is called once per frame.
inline GW::HookEntry Update_Entry;

class GWSM
{

public:
    // Delete copy constructor and operator
    GWSM(const GWSM&) = delete;
    void operator=(const GWSM&) = delete;

    static GWSM& Instance()
    {
        static GWSM instance;
        return instance;
    }

    void Init();
    void Terminate();

    // Must be static because it is called from a hooked function.
    static void Update(GW::HookStatus*);

    bool GW_is_closing = false;
    bool has_freed_resources = false;

    ClientDataUpdater client_data_updater_;
    ItemCallbacks item_callsbacks_;

private:
    GWSM()
        : connection_manager_(wstr_to_str(GW::GetCharContext()->player_email))
        , client_data_updater_(wstr_to_str(GW::GetCharContext()->player_email)){};

    GWIPC::ConnectionManager connection_manager_;
};
