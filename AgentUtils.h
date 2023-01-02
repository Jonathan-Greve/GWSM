#pragma once
namespace GW
{
struct AgentLiving;
}

bool is_hostile(const GW::AgentLiving* living);

std::vector<uint32_t> get_party_agent_ids();

uint32_t get_agent_id_from_party_slot(const uint32_t party_slot);

uint32_t get_party_slot_from_agent_id(const uint32_t agent_id);
