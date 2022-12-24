#pragma once

#include <GWCA/Utilities/Export.h>

namespace GW {

    typedef uint32_t PlayerNumber;

    struct Player;
    struct Quest;
    struct Title;
    struct TitleClientData;
    typedef Array<Quest> QuestLog;
    typedef Array<Player> PlayerArray;

    namespace Constants {
        enum class TitleID : uint32_t;
        enum class Profession;
        enum class QuestID;
    }

    struct Module;
    extern Module PlayerModule;

    namespace PlayerMgr {

        GWCA_API bool SetActiveTitle(Constants::TitleID title_id);

        GWCA_API bool RemoveActiveTitle();

        GWCA_API uint32_t GetPlayerAgentId(uint32_t player_id);

        GWCA_API uint32_t GetAmountOfPlayersInInstance();

        GWCA_API PlayerArray* GetPlayerArray();

        // Get the player number of the currently logged in character
        GWCA_API PlayerNumber GetPlayerNumber();

        GWCA_API Player *GetPlayerByID(uint32_t player_id = 0);

        GWCA_API wchar_t *GetPlayerName(uint32_t player_id = 0);

        GWCA_API wchar_t* SetPlayerName(uint32_t player_id, const wchar_t *replace_name);

        GWCA_API bool ChangeSecondProfession(Constants::Profession prof, uint32_t hero_index = 0);

        GWCA_API Player *GetPlayerByName(const wchar_t *name);

        GWCA_API QuestLog* GetQuestLog();

        GWCA_API GW::Constants::QuestID GetActiveQuestId();

        GWCA_API Quest* GetActiveQuest();

        // Get the current progress of a title by id. If the title has no progress, this function will return nullptr
        GWCA_API Title* GetTitleTrack(Constants::TitleID title_id);

        GWCA_API Constants::TitleID GetActiveTitleId();

        GWCA_API Title* GetActiveTitle();

        GWCA_API TitleClientData* GetTitleData(Constants::TitleID title_id);

        // Donate 5000 faction points to Luxon (1) or Kurzick (0). Must be talking to faction NPC.
        GWCA_API bool DepositFaction(uint32_t allegiance);
    };
}
