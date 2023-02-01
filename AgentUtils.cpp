#include "pch.h"
#include "AgentUtils.h"

bool is_hostile(const GW::AgentLiving* living)
{
    if (living->allegiance == GW::Constants::Allegiance::Enemy)
    {
        return true;
    }
    return false;
}

std::vector<uint32_t> get_party_agent_ids()
{
    GW::PartyInfo* party = GW::PartyMgr::GetPartyInfo();
    std::vector<uint32_t> party_agent_ids;

    if (party)
    {
        for (uint32_t i = 0; i < party->players.m_size; i++)
        {
            const auto& [login_number, calledTargetId, state] = party->players[i];
            if (const auto member_player = GW::Agents::GetPlayerByID(login_number))
            {
                party_agent_ids.push_back(member_player->agent_id);

                for (uint32_t j = 0; j < party->heroes.m_size; j++)
                {
                    if (party->heroes[j].owner_player_id == login_number)
                    {
                        party_agent_ids.push_back(party->heroes[j].agent_id);
                    }
                }
            }
        }
        for (uint32_t i = 0; i < party->henchmen.m_size; i++)
        {
            party_agent_ids.push_back(party->henchmen[i].agent_id);
        }

        // The ordering of these differ between the players in the party.
        for (uint32_t i = 0; i < party->others.m_size; i++)
        {
            party_agent_ids.push_back(party->others[i]);
        }
    }
    return party_agent_ids;
}

uint32_t get_agent_id_from_party_slot(const uint32_t party_slot)
{
    const auto party_agent_ids = get_party_agent_ids();
    return party_agent_ids[party_slot];
}

uint32_t get_party_slot_from_agent_id(const uint32_t agent_id)
{
    const auto party_agent_ids = get_party_agent_ids();
    uint32_t party_slot = 0;
    for (uint32_t i = 0; i < party_agent_ids.size(); i++)
    {
        if (party_agent_ids[i] == agent_id)
        {
            party_slot = i;
            break;
        }
    }
    return party_slot;
}
