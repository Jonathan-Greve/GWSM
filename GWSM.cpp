#include "pch.h"
#include "GWSM.h"

#include "KeyboardProc.h"
#include "SafeWndProc.h"

void GWSM::Init()
{
    ChatWriter::WriteIngameDebugChat("Init: Called.", ChatColor::Green);
    // Set up our own NewWndHandle and store a copy of the default WndProc in DefaultWndProc
    // We can then later restore current_GW_window_handle to be the default when we terminate.
    current_GW_window_handle = GW::MemoryMgr::GetGWWindowHandle();
    DefaultWndProc =
      SetWindowLongPtrW(current_GW_window_handle, GWL_WNDPROC, reinterpret_cast<long>(SafeWndProc));

    // Register our Update method to be called on each frame from within the game thread.
    // Note that the game thread is separate from the current thread. It is thE thread
    // controlled by the GW client.
    GW::GameThread::RegisterGameThreadCallback(&Update_Entry, Update);

    keyboard_hook_handle = SetWindowsHookExA(WH_KEYBOARD, &KeyboardProc, nullptr, GetCurrentThreadId());

    if (keyboard_hook_handle == NULL)
    {
        ChatWriter::WriteIngameDebugChat("Init: Failed setting keyboard hook.", ChatColor::DarkRed);
        Terminate();
    }

    if (! item_callsbacks_.init())
    {
        ChatWriter::WriteIngameDebugChat("Init: Failed initializing ItemCallbacks.", ChatColor::DarkRed);
        Terminate();
    }

    connection_manager_.connect();

    ChatWriter::WriteIngameDebugChat("Init: Finished.", ChatColor::Green);
}

// Remove all hooks. Free all resources. Disconnect any connections to external processes.
void GWSM::Terminate()
{
    ChatWriter::WriteIngameDebugChat("Terminate: Called.", ChatColor::Blue);
    if (! has_freed_resources)
    {
        connection_manager_.disconnect();
        connection_manager_.terminate();
        GW::GameThread::RemoveGameThreadCallback(&Update_Entry);

        UnhookWindowsHookEx(keyboard_hook_handle);

        // Restore the window handle to be the default one that GW launched with.
        SetWindowLongPtr(current_GW_window_handle, GWL_WNDPROC, DefaultWndProc);

        // Let ThreadEntry know that it can finish terminating our dll thread.
        has_freed_resources = true;
        ChatWriter::WriteIngameDebugChat("Terminate: Freed resources.", ChatColor::Blue);

        // If terminate was called because the window is closing (i.e. Alt-f4 or pressed close)
        // Then resend the WM_CLOSE signal that we intercepted earlier in NewWndProc.
        if (GW_is_closing)
        {
            SendMessageW(current_GW_window_handle, WM_CLOSE, NULL, NULL);
        }
    }

    ChatWriter::WriteIngameDebugChat("Terminate: Finished.", ChatColor::Blue);
}

//static int test = 1;

void GWSM::Update(GW::HookStatus*)
{
    if (! GWSM::Instance().has_freed_resources && ! GWSM::Instance().GW_is_closing)
    {
        static DWORD last_tick_count;
        if (last_tick_count == 0)
            last_tick_count = GetTickCount();

        const DWORD tick = GetTickCount();
        const DWORD delta = tick - last_tick_count;
        const float dt = static_cast<float>(delta) / 1000.f;

        last_tick_count = tick;

        const auto instance_type = GW::Map::GetInstanceType();
        const auto pregame_context = GW::GetPreGameContext();
        const auto cam = GW::CameraMgr::GetCamera();
        auto& gwsm_instance = GWSM::Instance();
        UpdateStatus update_status;
        if (pregame_context != nullptr)
        {
            // We are on the character select screen.
            update_status.game_state = GWIPC::GameState::GameState_CharSelect;
        }
        else if (instance_type == GW::Constants::InstanceType::Loading)
        {
            // In load screen.
            update_status.game_state = GWIPC::GameState::GameState_Loading;
        }
        else if (cam && ! std::isinf(cam->position.x))
        {
            // Always keep quest log open
            GW::GameThread::Enqueue(
              [&]()
              {
                  auto window_pos = GetWindowPosition(GW::UI::WindowID::WindowID_QuestLog);
                  if (! window_pos->visible())
                      GW::UI::Keypress(GW::UI::ControlAction_OpenQuestLog);
              });

            update_status.game_state = GWIPC::GameState::GameState_InGame;
        }
        else
        {
            update_status.game_state = GWIPC::GameState::GameState_Unknown;
        }

        gwsm_instance.client_data_updater_.update(
          update_status, gwsm_instance.item_callsbacks_.inventory_or_equipment_changed);

        // Set to false so that client_data_updater won't keep updating the same non-changed data.
        gwsm_instance.item_callsbacks_.inventory_or_equipment_changed = false;
    }
}
