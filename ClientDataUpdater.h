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
        flatbuffers::FlatBufferBuilder builder(GWIPC::CLIENTDATA_SIZE);

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
            build_agent_living(character_agent, agent_living_builder);
        }

        if (! name.IsNull())
            agent_living_builder.add_name(name);

        auto agent_living = agent_living_builder.Finish();
        character = GWIPC::CreateCharacter(builder, agent_living);
    }

    void build_agent_living(GW::AgentLiving* living_agent, GWIPC::AgentLivingBuilder& agent_living_builder)
    {
        auto position = GWIPC::Vec3(living_agent->x, -living_agent->z, living_agent->y);
        auto terrain_normal = GWIPC::Vec3(living_agent->terrain_normal.x, -living_agent->terrain_normal.z,
                                          living_agent->terrain_normal.y);
        auto velocity = GWIPC::Vec2(living_agent->velocity.x, living_agent->velocity.y);

        GWIPC::Agent agent(living_agent->agent_id, position, terrain_normal, living_agent->rotation_angle,
                           velocity, living_agent->width1, living_agent->height1, living_agent->timer);

        agent_living_builder.add_agent(&agent);
        agent_living_builder.add_animation_type(living_agent->animation_type);
        agent_living_builder.add_energy(living_agent->energy);
        agent_living_builder.add_energy_recharge(living_agent->energy_regen);
        agent_living_builder.add_guild_id(living_agent->tags->guild_id);
        agent_living_builder.add_health(living_agent->hp);
        agent_living_builder.add_health_recharge(living_agent->hp_pips);
        agent_living_builder.add_level(living_agent->level);
        agent_living_builder.add_max_energy(living_agent->max_energy);
        agent_living_builder.add_max_health(living_agent->max_hp);
        agent_living_builder.add_owner_agent_id(living_agent->owner);
        agent_living_builder.add_player_number(living_agent->player_number);
        agent_living_builder.add_primary_profession(static_cast<GWIPC::Profession>(living_agent->primary));
        agent_living_builder.add_secondary_profession(
          static_cast<GWIPC::Profession>(living_agent->secondary));
        agent_living_builder.add_team_color(static_cast<GWIPC::TeamColor>(living_agent->team_id));
        agent_living_builder.add_weapon_attack_speed(living_agent->weapon_attack_speed);
        agent_living_builder.add_weapon_attack_speed_modifier(living_agent->attack_speed_modifier);
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

        auto party_context = GW::GetPartyContext();
        if (party_context)
        {
            auto player_party = party_context->player_party;
            if (player_party)
            {

                std::vector<flatbuffers::Offset<GWIPC::Hero>> heroes_vector;
                for (const auto& hero : player_party->heroes)
                {
                    flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Skill*>> skills_vector;
                    const auto skillbar_array = GW::SkillbarMgr::GetSkillbarArray();
                    if (skillbar_array)
                    {
                        for (const auto& hero_skillbar : *skillbar_array)
                        {
                            if (hero_skillbar.agent_id == hero.agent_id)
                            {
                                std::vector<GWIPC::Skill> skills;
                                for (const auto& skill : hero_skillbar.skills)
                                {
                                    GWIPC::Skill new_skill((uint16_t)skill.skill_id,
                                                           (uint16_t)skill.GetRecharge(),
                                                           (uint8_t)skill.adrenaline_a);
                                    skills.push_back(new_skill);
                                }
                                skills_vector = builder.CreateVectorOfStructs(skills);
                                break;
                            }
                        }
                    }

                    GWIPC::SkillbarBuilder skillbar_builder(builder);
                    skillbar_builder.add_skills(skills_vector);
                    auto skillbar = skillbar_builder.Finish();

                    flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Effect*>> hero_effects_vector;
                    flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Buff*>> hero_buffs_vector;
                    const auto agent_effects = GW::Effects::GetAgentEffectsArray(hero.agent_id);
                    if (agent_effects)
                    {

                        const auto& effects = agent_effects->effects;
                        if (effects.valid())
                        {
                            std::vector<GWIPC::Effect> effects_vector;
                            for (const auto& effect : effects)
                            {
                                GWIPC::Effect new_effect((uint32_t)effect.skill_id, effect.agent_id,
                                                         (float)effect.duration);
                                effects_vector.push_back(new_effect);
                            }
                            hero_effects_vector = builder.CreateVectorOfStructs(effects_vector);
                        }

                        const auto& buffs = agent_effects->buffs;
                        if (buffs.valid())
                        {
                            std::vector<GWIPC::Buff> buffs_vector;
                            for (const auto& buff : buffs)
                            {
                                GWIPC::Buff new_buff((uint32_t)buff.skill_id, buff.target_agent_id,
                                                     (uint32_t)buff.buff_id);
                                buffs_vector.push_back(new_buff);
                            }
                            hero_buffs_vector = builder.CreateVectorOfStructs(buffs_vector);
                        }
                    }

                    flatbuffers::Offset<GWIPC::AgentLiving> agent_living;
                    auto hero_agent = GW::Agents::GetAgentByID(hero.agent_id);
                    if (hero_agent)
                    {
                        auto hero_agent_living = hero_agent->GetAsAgentLiving();
                        if (hero_agent_living)
                        {
                            GWIPC::AgentLivingBuilder agent_living_builder(builder);
                            build_agent_living(hero_agent_living, agent_living_builder);
                            agent_living_builder.add_party_slot(
                              get_party_slot_from_agent_id(hero_agent->agent_id));

                            agent_living = agent_living_builder.Finish();
                        }
                    }
                    GWIPC::Vec2 flag_pos;
                    const auto world_context = GW::GetWorldContext();
                    if (world_context)
                    {
                        const auto& flags = world_context->hero_flags;
                        if (flags.valid())
                        {
                            for (const auto& flag : flags)
                            {
                                if (flag.hero_id == hero.hero_id)
                                {
                                    flag_pos = GWIPC::Vec2(flag.flag.x, flag.flag.y);
                                    break;
                                }
                            }
                        }
                    }

                    GWIPC::HeroBuilder hero_builder(builder);
                    hero_builder.add_agent_living(agent_living);
                    hero_builder.add_skillbar(skillbar);
                    hero_builder.add_effects(hero_effects_vector);
                    hero_builder.add_buffs(hero_buffs_vector);
                    hero_builder.add_flag_position(&flag_pos);

                    hero_builder.add_owner_player_id(hero.owner_player_id);
                    hero_builder.add_hero_id(hero.hero_id);
                    auto fb_hero = hero_builder.Finish();
                    heroes_vector.push_back(fb_hero);
                }

                auto heroes = builder.CreateVector(heroes_vector);

                GWIPC::PartyBuilder party_builder(builder);
                party_builder.add_party_id(player_party->party_id);
                party_builder.add_hero_members(heroes);

                const auto world_context = GW::GetWorldContext();
                if (world_context)
                {
                    const auto& flag = world_context->all_flag;
                    GWIPC::Vec3 all_flag_pos(flag.x, -flag.z, flag.y);
                    party_builder.add_flag_all_position(&all_flag_pos);
                }
                party = party_builder.Finish();
            }
        }
    }
};
