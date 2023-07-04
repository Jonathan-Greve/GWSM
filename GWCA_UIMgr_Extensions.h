#pragma once
namespace GW
{
bool InitUIExtensions();
namespace UI
{
    GWCA_API bool ChangeQuest(uint32_t new_quest_id);
}
namespace Render {
    GWCA_API void SetShouldRender(bool new_value);
}
}
