#pragma once
#include "SharedMemory.h"
#include "clientData_generated.h"

class ClientDataUpdater
{
public:
    ClientDataUpdater(std::string email)
        : shared_memory_(email, GWIPC::CLIENTDATA_SIZE)
    {
    }

    void update(UpdateStatus update_status)
    {
        // Create a flatbuffer builder object
        flatbuffers::FlatBufferBuilder builder;

        // Create the Character object
        flatbuffers::Offset<GWIPC::Character> character;
        build_character(builder, character);

        // Create the Instance object
        flatbuffers::Offset<GWIPC::Instance> instance;
        build_instance(builder, instance);

        // Create the Party object
        flatbuffers::Offset<GWIPC::Party> party;
        build_party(builder, party);

        // Create the ClientData object
        auto client_data =
          GWIPC::CreateClientData(builder, character, instance, party, update_status.game_state);

        // Finish creating the flatbuffer and retrieve a pointer to the buffer
        builder.Finish(client_data);

        // Get pointer to buffer and size
        uint8_t* buf = builder.GetBufferPointer();
        int size = builder.GetSize();

        shared_memory_.write_data(buf, size);
    }

private:
    GWIPC::SharedMemory shared_memory_;

    void build_character(flatbuffers::FlatBufferBuilder& builder,
                         flatbuffers::Offset<GWIPC::Character>& character)
    {

        auto character_context = GW::GetCharContext();

        flatbuffers::Offset<flatbuffers::String> name;
        if (character_context)
        {
            auto char_name = wstr_to_str(character_context->player_name);
            name = builder.CreateString(char_name.c_str());
        }

        GWIPC::AgentLivingBuilder agent_living_builder(builder);

        auto character_agent = GW::Agents::GetCharacter();
        if (character_agent)
        {
            auto position = GWIPC::Vec3(character_agent->x, -character_agent->z, character_agent->y);
            auto terrain_normal =
              GWIPC::Vec3(character_agent->terrain_normal.x, -character_agent->terrain_normal.z,
                          character_agent->terrain_normal.y);
            auto velocity = GWIPC::Vec2(character_agent->velocity.x, character_agent->velocity.y);

            GWIPC::Agent agent(character_agent->agent_id, position, terrain_normal,
                               character_agent->rotation_angle, velocity, character_agent->width1,
                               character_agent->height1, character_agent->timer);

            agent_living_builder.add_agent(&agent);
            agent_living_builder.add_animation_type(character_agent->animation_type);
            agent_living_builder.add_energy(character_agent->energy);
            agent_living_builder.add_energy_recharge(character_agent->energy_regen);
            agent_living_builder.add_guild_id(character_agent->tags->guild_id);
            agent_living_builder.add_health(character_agent->hp);
            agent_living_builder.add_health_recharge(character_agent->hp_pips);
            agent_living_builder.add_level(character_agent->level);
            agent_living_builder.add_max_energy(character_agent->max_energy);
            agent_living_builder.add_max_health(character_agent->max_hp);
            agent_living_builder.add_owner_agent_id(character_agent->owner);
            agent_living_builder.add_player_number(character_agent->player_number);
            agent_living_builder.add_primary_profession(
              static_cast<GWIPC::Profession>(character_agent->primary));
            agent_living_builder.add_secondary_profession(
              static_cast<GWIPC::Profession>(character_agent->secondary));
            agent_living_builder.add_team_color(static_cast<GWIPC::TeamColor>(character_agent->team_id));
            agent_living_builder.add_weapon_attack_speed(character_agent->weapon_attack_speed);
            agent_living_builder.add_weapon_attack_speed_modifier(character_agent->attack_speed_modifier);
        }

        if (! name.IsNull())
            agent_living_builder.add_name(name);

        auto agent_living = agent_living_builder.Finish();
        character = GWIPC::CreateCharacter(builder, agent_living);
    }

    void build_instance(flatbuffers::FlatBufferBuilder& builder,
                        flatbuffers::Offset<GWIPC::Instance>& instance)
    {

        GWIPC::InstanceBuilder instance_builder(builder);
        auto character_context = GW::GetCharContext();
        if (character_context)
        {
            instance_builder.add_instance_id(character_context->token1);
            instance_builder.add_map_id(character_context->map_id);
        }

        instance = instance_builder.Finish();
    }

    void build_party(flatbuffers::FlatBufferBuilder& builder, flatbuffers::Offset<GWIPC::Party>& party)
    {
        GWIPC::PartyBuilder party_builder(builder);

        auto party_context = GW::GetPartyContext();
        if (party_context)
        {
            auto player_party = party_context->player_party;
            if (player_party)
            {
                party_builder.add_party_id(player_party->party_id);
            }
        }

        party = party_builder.Finish();
    }
};
