#include "pch.h"
#include "GWCA_UIMgr_Extensions.h"

uintptr_t UI_Windows_structs;
uintptr_t should_render_address;

typedef void(__thiscall* ChangeActiveQuestTextAndMarker_pt)(void* ecx_this_ptr, uint32_t* change_quest_struct,
                                                            bool should_change_highlighted_quest);
ChangeActiveQuestTextAndMarker_pt ChangeActiveQuestTextAndMarker_Func = 0;

typedef void(__cdecl* ChangeActiveQuest_pt)(uint32_t quest_id);
ChangeActiveQuest_pt ChangeActiveQuest_Func = 0;

namespace GW
{
bool InitUIExtensions()
{
    uintptr_t address =
      Scanner::Find("\x83\xc4\x08\xc7\x85\xe8\xfe\xff\xff\x00\x00\x00\x00\x8d\x85", "xxxxxxxxxxxxxxx", -9);
    if (address)
        UI_Windows_structs = *(uintptr_t*)address;

    ChangeActiveQuestTextAndMarker_Func = (ChangeActiveQuestTextAndMarker_pt)GW::Scanner::Find(
      "\x55\x8b\xec\x83\xec\x08\x83\x7d\x0c\x00\x53\x56", "xxxxxxxxxxxx", 0);

    ChangeActiveQuest_Func = (ChangeActiveQuest_pt)GW::Scanner::Find(
      "\x55\x8b\xec\x83\xec\x1c\x53\x56\x57\xe8\x82", "xxxxxxxxxxx", 0);

    should_render_address =
      GW::Scanner::Find("\xD8\xD1\xDF\xE0\x56\x8B\x75\x08\xDD\xD9\xF6\xC4\x41\x0F\x8B\x76\x02\x00\x00",
                        "xxxxxxxxxxxxxxxxxxx", +0x74);

    if (ChangeActiveQuestTextAndMarker_Func && ChangeActiveQuest_Func && should_render_address)
    {
        return true;
    }

    return false;
}

namespace UI
{

    bool ChangeQuest(uint32_t new_quest_id)
    {
        // Find quest log ptr
        if (! IsBadReadPtr((const void*)UI_Windows_structs, sizeof(uint32_t)))
        {
            // Number of open UI windows
            uint32_t number_of_UI_windows = *(uint32_t*)(UI_Windows_structs + 8);

            // Find the struct for the quest log window.
            for (uint32_t ui_index = 0; ui_index < number_of_UI_windows; ui_index++)
            {
                uintptr_t quest_log_ptr = *((uint32_t*)UI_Windows_structs);
                if (! IsBadReadPtr((const void*)quest_log_ptr, sizeof(uint32_t)))
                {
                    quest_log_ptr = *((uint32_t*)(quest_log_ptr + ui_index * 4));
                    if (! IsBadReadPtr((const void*)quest_log_ptr, sizeof(uint32_t)))
                    {
                        // This offset (0x48) uniquely identifies thet type of window. 0x25c=questlog, 0x249=skill&attrib window, ...
                        bool is_quest_window = *((uint32_t*)(quest_log_ptr + 0x48)) == 0x25c;
                        if (is_quest_window)
                        {
                            quest_log_ptr = *((uint32_t*)(quest_log_ptr + 0xA0));
                            if (! IsBadReadPtr((const void*)quest_log_ptr, sizeof(uint32_t)))
                            {
                                quest_log_ptr = *((uint32_t*)(quest_log_ptr + 4));
                                if (! IsBadReadPtr((const void*)quest_log_ptr, sizeof(uint32_t)))
                                {
                                    if (ChangeActiveQuestTextAndMarker_Func)
                                    {
                                        uint32_t change_quest_struct[] = {0, new_quest_id};

                                        ChangeActiveQuestTextAndMarker_Func((void*)quest_log_ptr,
                                                                            change_quest_struct, true);
                                        //ChangeActiveQuest_Func(new_quest_id);
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

}
namespace Render
{
    GWCA_API void SetShouldRender(bool new_value)
    {
        **reinterpret_cast<bool**>(should_render_address) = new_value;
    }
}
}
