#pragma once
#include "SharedMemory.h"
#include "clientData_generated.h"

class ClientDataUpdater
{
public:
    ClientDataUpdater(std::string email)
        : shared_memory_(email, GWIPC::CLIENTDATA_SIZE)
        , builder_(GWIPC::CLIENTDATA_SIZE)
        , buffer_(GWIPC::CLIENTDATA_SIZE)
    {
    }

    void update(const UpdateStatus update_status, const GWIPC::UpdateOptions* update_options,
                const bool inventory_or_equipment_changed, const bool quests_changed,
                std::string& nav_mesh_file_path, DialogsManager& dialogs_manager)
    {
        const auto quests_decoding_size = current_quest_strs_decoding.size();
        const auto bag_items_decoding_size = current_bag_item_strs_decoding.size();

        // Create a flatbuffer builder object
        builder_.Clear();

        // Create the Character object
        flatbuffers::Offset<GWIPC::Character> character;
        build_character(builder_, character);

        // Create the Instance object
        flatbuffers::Offset<GWIPC::Instance> instance;
        build_instance(builder_, instance);

        // Create the Party object
        flatbuffers::Offset<GWIPC::Party> party;
        build_party(builder_, party);

        auto current_time = std::chrono::system_clock::now();
        auto elapsed_time =
          std::chrono::duration_cast<std::chrono::seconds>(current_time - last_build_quests_time_).count();

        flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<GWIPC::Quest>>> quests;
        if (update_status.game_state == GWIPC::GameState::GameState_InGame)
        {
            if (buffer_.empty() || quests_changed || quests_decoding_size > 0 || elapsed_time >= 60)
            {
                if (quests_changed)
                {
                    quest_strs_.clear();
                    already_called_changequest_ids.clear();
                }

                build_quests(builder_, quests);
                last_build_quests_time_ = std::chrono::system_clock::now();
            }
            else
            {
                auto active_quest_id = static_cast<uint32_t>(GW::PlayerMgr::GetActiveQuestId());

                const auto client_data = GWIPC::GetClientData(buffer_.data());
                if (client_data)
                {
                    auto cached_quests = client_data->quests();
                    if (cached_quests)
                    {
                        std::vector<flatbuffers::Offset<GWIPC::Quest>> quests_vector;
                        for (uint32_t i = 0; i < cached_quests->size(); i++)
                        {
                            auto cached_quest = cached_quests->Get(i);
                            if (cached_quest)
                            {
                                GWIPC::Vec3 marker;
                                if (cached_quest->marker())
                                {
                                    marker =
                                      GWIPC::Vec3(cached_quest->marker()->x(), cached_quest->marker()->y(),
                                                  cached_quest->marker()->z());
                                }

                                std::string description = "";
                                if (! update_options->only_send_active_quest_description() ||
                                    cached_quest->quest_id() == active_quest_id)
                                {
                                    description = cached_quest->description()->str();
                                }
                                std::string objectives = "";
                                if (! update_options->only_send_active_quest_objectives() ||
                                    cached_quest->quest_id() == active_quest_id)
                                {
                                    objectives = cached_quest->objectives()->str();
                                }

                                auto new_quest = create_quest_from_values(
                                  builder_, description, cached_quest->location()->str(),
                                  cached_quest->name()->str(), cached_quest->npc_name()->str(), objectives,
                                  cached_quest->log_state(), cached_quest->map_from(), cached_quest->map_to(),
                                  marker, cached_quest->quest_id());

                                quests_vector.emplace_back(new_quest);
                            }
                        }
                        quests = builder_.CreateVector(quests_vector);
                    }
                }
            }
        }

        // Sets the 5 bags (regular bags 1-4 + equipment bag).
        // Tries to use the previous flatbuffer as a cache for the bag items.
        // This is because it is very slow to rebuild from scratch.
        flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<GWIPC::Bag>>> bags;
        // Check the elapsed time since build_bags was last called
        elapsed_time =
          std::chrono::duration_cast<std::chrono::seconds>(current_time - last_build_bags_time_).count();
        if (buffer_.empty() || inventory_or_equipment_changed || bag_items_decoding_size > 0 ||
            elapsed_time >= 60)
        {
            if (inventory_or_equipment_changed)
            {
                bag_item_strs_.clear();
            }

            build_bags(builder_, bags);
            last_build_bags_time_ = std::chrono::system_clock::now();
        }
        else
        {
            const auto client_data = GWIPC::GetClientData(buffer_.data());
            if (client_data)
            {
                const auto cached_bags = client_data->bags();
                if (cached_bags)
                {
                    for (uint32_t i = 0; i < cached_bags->size(); i++)
                    {
                        std::vector<flatbuffers::Offset<GWIPC::Bag>> bags_vector;
                        const GWIPC::Bag* bag = cached_bags->Get(i);
                        if (bag)
                        {
                            auto items = bag->items();
                            if (items)
                            {
                                std::vector<flatbuffers::Offset<GWIPC::BagItem>> bag_items_vector;
                                for (uint32_t j = 0; j < items->size(); j++)
                                {
                                    auto item = items->Get(j);
                                    if (item)
                                    {
                                        auto new_bag_item = create_bag_item_from_values(
                                          builder_, item->description()->str(), item->full_name()->str(),
                                          item->single_item_name()->str(), item->name()->str(),
                                          item->interaction(), item->is_weapon_set_item(), item->item_id(),
                                          item->item_modifier(), item->model_id(), item->quantity(),
                                          item->type(), item->value(), item->index());

                                        bag_items_vector.emplace_back(new_bag_item);
                                    }
                                }
                                auto bag_items = builder_.CreateVector(bag_items_vector);
                                GWIPC::BagBuilder bag_builder(builder_);
                                bag_builder.add_items(bag_items);
                                bag_builder.add_bag_size(static_cast<uint8_t>(bag->bag_size()));

                                auto new_bag = bag_builder.Finish();
                                bags_vector.emplace_back(new_bag);
                            }
                        }
                        bags = builder_.CreateVector(bags_vector);
                    }
                }
            }
        }

        flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<GWIPC::BagItem>>> equipped_items;
        elapsed_time =
          std::chrono::duration_cast<std::chrono::seconds>(current_time - last_build_equipped_items_time_)
            .count();
        if (buffer_.empty() || inventory_or_equipment_changed || bag_items_decoding_size > 0 ||
            elapsed_time >= 60)
        {
            build_equipped_items(builder_, equipped_items);
            last_build_equipped_items_time_ = std::chrono::system_clock::now();
        }
        else
        {
            const auto client_data = GWIPC::GetClientData(buffer_.data());
            if (client_data)
            {
                const auto items_equipped = client_data->items_equiped();
                if (items_equipped)
                {
                    std::vector<flatbuffers::Offset<GWIPC::BagItem>> bag_items_vector;
                    for (uint32_t j = 0; j < items_equipped->size(); j++)
                    {
                        auto item = items_equipped->Get(j);
                        if (item)
                        {

                            auto new_bag_item = create_bag_item_from_values(
                              builder_, item->description()->str(), item->full_name()->str(),
                              item->single_item_name()->str(), item->name()->str(), item->interaction(),
                              item->is_weapon_set_item(), item->item_id(), item->item_modifier(),
                              item->model_id(), item->quantity(), item->type(), item->value(), item->index());

                            bag_items_vector.emplace_back(new_bag_item);
                        }
                    }
                    equipped_items = builder_.CreateVector(bag_items_vector);
                }
            }
        }

        flatbuffers::Offset<GWIPC::DialogsInfo> dialogs_info;
        build_dialogs_info(builder_, dialogs_info, dialogs_manager);

        auto nav_mesh_file_path_fb = builder_.CreateString(nav_mesh_file_path);

        // Create the ClientData object
        auto client_data =
          GWIPC::CreateClientData(builder_, character, instance, party, update_status.game_state, quests,
                                  bags, equipped_items, dialogs_info, nav_mesh_file_path_fb);

        // Finish creating the flatbuffer and retrieve a pointer to the buffer
        builder_.Finish(client_data);

        // Get pointer to buffer and size
        buffer_ =
          std::vector<uint8_t>(builder_.GetBufferPointer(), builder_.GetBufferPointer() + builder_.GetSize());
        int size = builder_.GetSize();

        shared_memory_.write_data(buffer_.data(), size);

        if (update_status.game_state != GWIPC::GameState_InGame)
        {
            buffer_.clear();
        }
    }

private:
    GWIPC::SharedMemory shared_memory_;

    flatbuffers::FlatBufferBuilder builder_;

    std::vector<uint8_t> buffer_;
    std::chrono::time_point<std::chrono::system_clock> last_build_quests_time_;
    std::chrono::time_point<std::chrono::system_clock> last_build_bags_time_;
    std::chrono::time_point<std::chrono::system_clock> last_build_equipped_items_time_;

    struct QuestStrings
    {
        std::wstring location = L"";
        std::wstring name = L"";
        std::wstring npc_name = L"";
        std::wstring description = L"";
        std::wstring objectives = L"";
    };

    struct BagItemStrings
    {
        std::wstring name = L"";
        std::wstring single_item_name = L"";
        std::wstring full_name = L"";
        std::wstring description = L"";
    };

    // objective_id => wstring;
    std::unordered_map<uint32_t, std::wstring> mission_objectives_strs_;
    // quest_id => QuestString;
    std::unordered_map<uint32_t, QuestStrings> quest_strs_;
    std::set<uint32_t> current_quest_strs_decoding;
    // (model_id, mod) => BagItemStrings;
    std::map<std::pair<uint32_t, uint32_t>, BagItemStrings> bag_item_strs_;
    std::set<std::pair<uint32_t, uint32_t>> current_bag_item_strs_decoding;

    std::set<uint32_t> already_called_changequest_ids;

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

        auto character_agent = GW::Agents::GetCharacter();
        flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Effect*>> effects_vector;
        flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Buff*>> buffs_vector;
        flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Skill*>> skills_vector;
        if (character_agent)
        {
            create_buff_and_effect_vectors(builder, character_agent->agent_id, effects_vector, buffs_vector);
            create_skills_vector(builder, character_agent->agent_id, skills_vector);
        }
        GWIPC::SkillbarBuilder skillbar_builder(builder);
        skillbar_builder.add_skills(skills_vector);
        auto skillbar = skillbar_builder.Finish();

        GWIPC::AgentLivingBuilder agent_living_builder(builder);
        if (character_agent)
        {
            build_agent_living(character_agent, agent_living_builder);
            agent_living_builder.add_party_slot(get_party_slot_from_agent_id(character_agent->agent_id));
        }

        if (! name.IsNull())
            agent_living_builder.add_name(name);

        auto agent_living = agent_living_builder.Finish();

        const auto target_agent_id = GW::Agents::GetTargetId();

        character = GWIPC::CreateCharacter(builder, agent_living, skillbar, effects_vector, buffs_vector,
                                           target_agent_id);
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

        auto instance_type = GW::Map::GetInstanceType();
        if (instance_type != GW::Constants::InstanceType::Loading)
        {
            instance_builder.add_map_instance_type(static_cast<GWIPC::MapInstanceType>(instance_type));
        }

        instance = instance_builder.Finish();
    }

    void create_buff_and_effect_vectors(
      flatbuffers::FlatBufferBuilder& builder, const uint32_t agent_id,
      flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Effect*>>& effects_vector_out,
      flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Buff*>>& buffs_vector_out)
    {
        const auto agent_effects = GW::Effects::GetAgentEffectsArray(agent_id);
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
                effects_vector_out = builder.CreateVectorOfStructs(effects_vector);
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
                buffs_vector_out = builder.CreateVectorOfStructs(buffs_vector);
            }
        }
    }

    void create_skills_vector(flatbuffers::FlatBufferBuilder& builder, const uint32_t agent_id,
                              flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Skill*>>& skills_vector)
    {
        const auto skillbar_array = GW::SkillbarMgr::GetSkillbarArray();
        if (skillbar_array)
        {
            for (const auto& skillbar : *skillbar_array)
            {
                if (skillbar.agent_id == agent_id)
                {
                    std::vector<GWIPC::Skill> skills;
                    for (const auto& skill : skillbar.skills)
                    {
                        GWIPC::Skill new_skill((uint16_t)skill.skill_id, (uint16_t)skill.GetRecharge(),
                                               (uint8_t)skill.adrenaline_a);
                        skills.push_back(new_skill);
                    }
                    skills_vector = builder.CreateVectorOfStructs(skills);
                    break;
                }
            }
        }
    }

    void build_party(flatbuffers::FlatBufferBuilder& builder, flatbuffers::Offset<GWIPC::Party>& party)
    {
        auto party_context = GW::GetPartyContext();
        if (party_context)
        {
            auto player_party = party_context->player_party;
            if (player_party)
            {
                std::vector<flatbuffers::Offset<GWIPC::AgentLiving>> players_vector;
                for (const auto& player : player_party->players)
                {
                    auto player_agent = GW::Agents::GetPlayerByID(player.login_number);
                    if (player_agent)
                    {
                        auto player_agent_living = player_agent->GetAsAgentLiving();
                        if (player_agent_living)
                        {
                            GWIPC::AgentLivingBuilder agent_living_builder(builder);
                            build_agent_living(player_agent_living, agent_living_builder);
                            agent_living_builder.add_party_slot(
                              get_party_slot_from_agent_id(player_agent->agent_id));

                            auto agent_living = agent_living_builder.Finish();

                            players_vector.push_back(agent_living);
                        }
                    }
                }

                std::vector<flatbuffers::Offset<GWIPC::AgentLiving>> henchmen_vector;
                for (const auto& henchman : player_party->henchmen)
                {
                    auto henchman_agent = GW::Agents::GetAgentByID(henchman.agent_id);
                    if (henchman_agent)
                    {
                        auto henchman_agent_living = henchman_agent->GetAsAgentLiving();
                        if (henchman_agent_living)
                        {
                            GWIPC::AgentLivingBuilder agent_living_builder(builder);
                            build_agent_living(henchman_agent_living, agent_living_builder);
                            agent_living_builder.add_party_slot(
                              get_party_slot_from_agent_id(henchman_agent->agent_id));

                            auto agent_living = agent_living_builder.Finish();

                            henchmen_vector.push_back(agent_living);
                        }
                    }
                }

                std::vector<flatbuffers::Offset<GWIPC::Hero>> heroes_vector;
                for (const auto& hero : player_party->heroes)
                {
                    flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Skill*>> skills_vector;
                    create_skills_vector(builder, hero.agent_id, skills_vector);

                    GWIPC::SkillbarBuilder skillbar_builder(builder);
                    skillbar_builder.add_skills(skills_vector);
                    auto skillbar = skillbar_builder.Finish();

                    flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Effect*>> hero_effects_vector;
                    flatbuffers::Offset<flatbuffers::Vector<const GWIPC::Buff*>> hero_buffs_vector;
                    create_buff_and_effect_vectors(builder, hero.agent_id, hero_effects_vector,
                                                   hero_buffs_vector);

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

                auto players = builder.CreateVector(players_vector);
                auto henchmen = builder.CreateVector(henchmen_vector);
                auto heroes = builder.CreateVector(heroes_vector);

                std::vector<flatbuffers::Offset<GWIPC::MissionObjective>> mission_objective_vector;
                const auto world_context = GW::GetWorldContext();
                if (world_context)
                {
                    const auto& mission_objectives = world_context->mission_objectives;
                    if (mission_objectives.valid())
                    {
                        for (const auto& mission_objective : mission_objectives)
                        {
                            const auto it = mission_objectives_strs_.find(mission_objective.objective_id);
                            if (it != mission_objectives_strs_.end())
                            {
                                if (it->second != L"")
                                {
                                    auto description = builder.CreateString(wstr_to_str(it->second.c_str()));

                                    auto mission_objective_builder = GWIPC::MissionObjectiveBuilder(builder);
                                    mission_objective_builder.add_objective_id(
                                      mission_objective.objective_id);
                                    mission_objective_builder.add_description(description);
                                    mission_objective_builder.add_type(mission_objective.type);

                                    auto new_mission_objective = mission_objective_builder.Finish();

                                    mission_objective_vector.emplace_back(new_mission_objective);
                                }
                            }
                            else
                            {
                                auto insert_it =
                                  mission_objectives_strs_.emplace(mission_objective.objective_id, L"");
                                GW::UI::AsyncDecodeStr(mission_objective.enc_str, &(*insert_it.first).second);
                            }
                        }
                    }
                }

                auto mission_objectives = builder.CreateVector(mission_objective_vector);

                GWIPC::PartyBuilder party_builder(builder);
                party_builder.add_party_id(player_party->party_id);
                party_builder.add_hero_members(heroes);
                party_builder.add_player_members(players);
                party_builder.add_henchman_members(henchmen);
                party_builder.add_mission_objectives(mission_objectives);

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

    void build_quests(flatbuffers::FlatBufferBuilder& builder,
                      flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<GWIPC::Quest>>>& quests)
    {
        const auto world_context = GW::GetWorldContext();
        if (world_context)
        {
            const auto& quest_log = world_context->quest_log;
            if (quest_log.valid())
            {
                std::vector<flatbuffers::Offset<GWIPC::Quest>> quests_vector;
                for (const auto& quest : quest_log)
                {
                    auto key = (uint32_t)quest.quest_id;
                    const auto it = quest_strs_.find(key);
                    if (it != quest_strs_.end())
                    {
                        if (it->second.location != L"" && it->second.name != L"" &&
                            it->second.description != L"" && it->second.npc_name != L"")
                        {
                            current_quest_strs_decoding.erase(key);

                            GWIPC::Vec3 marker(quest.marker.x, -quest.marker.z, quest.marker.y);

                            auto new_quest = create_quest_from_values(
                              builder, wstr_to_str(it->second.description.c_str()),
                              wstr_to_str(it->second.location.c_str()), wstr_to_str(it->second.name.c_str()),
                              wstr_to_str(it->second.npc_name.c_str()),
                              wstr_to_str(it->second.objectives.c_str()), quest.log_state, quest.map_from,
                              (uint32_t)quest.map_to, marker, (uint32_t)quest.quest_id);

                            quests_vector.emplace_back(new_quest);
                        }
                    }
                    else
                    {

                        if (quest.location && quest.name && quest.npc && quest.description &&
                            quest.objectives)
                        {
                            auto insert_it = quest_strs_.emplace(key, QuestStrings());
                            if (quest.description)
                            {
                                GW::UI::AsyncDecodeStr(quest.description,
                                                       &(*insert_it.first).second.description);
                            }
                            GW::UI::AsyncDecodeStr(quest.location, &(*insert_it.first).second.location);
                            GW::UI::AsyncDecodeStr(quest.name, &(*insert_it.first).second.name);
                            GW::UI::AsyncDecodeStr(quest.npc, &(*insert_it.first).second.npc_name);
                            if (quest.objectives && *quest.objectives != 0)
                            {
                                GW::UI::AsyncDecodeStr(quest.objectives,
                                                       &(*insert_it.first).second.objectives);
                            }

                            current_quest_strs_decoding.emplace(key);
                        }
                        else
                        {
                            if (! already_called_changequest_ids.contains(key))
                            {
                                auto active_quest_id = (uint32_t)GW::PlayerMgr::GetActiveQuestId();
                                if (active_quest_id != key)
                                {
                                    GW::UI::ChangeQuest(key);
                                    already_called_changequest_ids.insert(key);
                                }

                                ChatWriter::WriteIngameDebugChat(
                                  std::format("Init: Could not decode a quest: {}",
                                              static_cast<uint32_t>(quest.quest_id)),
                                  ChatColor::DarkRed);
                            }
                        }
                    }
                }

                quests = builder.CreateVector(quests_vector);
            }
        }
    }
    uint32_t create_bag_item(flatbuffers::FlatBufferBuilder& builder, GW::Bag** const& bag_array,
                             const uint32_t& bag_index,
                             std::vector<flatbuffers::Offset<GWIPC::BagItem>>& bag_items_vector)
    {
        uint32_t bag_size = 0;
        const auto bag = bag_array[bag_index];
        if (bag)
        {
            const auto& items = bag->items;
            if (items.valid())
            {
                bag_size = items.size();
                for (uint32_t item_index = 0; item_index < items.size(); item_index++)
                {
                    auto item = items[item_index];
                    if (! item)
                    {
                        // GetItemBySlot uses 1-index so we add 1;
                        item = GW::Items::GetItemBySlot(bag_index, item_index + 1);
                    }
                    if (item)
                    {
                        std::pair<uint32_t, uint32_t> key(item->model_id, item->mod_struct->mod);
                        const auto it = bag_item_strs_.find(key);
                        if (it != bag_item_strs_.end())
                        {
                            if ((! item->info_string || it->second.description != L"") &&
                                it->second.full_name != L"" && it->second.name != L"" &&
                                it->second.single_item_name != L"")
                            {
                                current_bag_item_strs_decoding.erase(key);

                                auto new_bag_item = create_bag_item_from_values(
                                  builder, wstr_to_str(it->second.description.c_str()),
                                  wstr_to_str(it->second.full_name.c_str()),
                                  wstr_to_str(it->second.single_item_name.c_str()),
                                  wstr_to_str(it->second.name.c_str()), item->interaction,
                                  is_weapon_set_item(item), item->item_id,
                                  item->mod_struct ? item->mod_struct->mod : 0, item->model_id,
                                  item->quantity, item->type, item->value, item_index);

                                bag_items_vector.emplace_back(new_bag_item);
                            }
                        }
                        else
                        {
                            if (item->name_enc && item->complete_name_enc && item->single_item_name)
                            {
                                auto insert_it = bag_item_strs_.emplace(key, BagItemStrings());
                                if (item->info_string)
                                    GW::UI::AsyncDecodeStr(item->info_string,
                                                           &(*insert_it.first).second.description);
                                GW::UI::AsyncDecodeStr(item->complete_name_enc,
                                                       &(*insert_it.first).second.full_name);
                                GW::UI::AsyncDecodeStr(item->name_enc, &(*insert_it.first).second.name);
                                GW::UI::AsyncDecodeStr(item->single_item_name,
                                                       &(*insert_it.first).second.single_item_name);
                                current_bag_item_strs_decoding.emplace(key);
                            }
                            else
                            {
                                ChatWriter::WriteIngameDebugChat(
                                  std::format("create_bag_item: Could not decode strings for item in bag: "
                                              "{}, index: {}.",
                                              bag_index, item_index),
                                  ChatColor::Blue);
                            }
                        }
                    }
                }
            }
        }
        return bag_size;
    }

    void build_bags(flatbuffers::FlatBufferBuilder& builder,
                    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<GWIPC::Bag>>>& bags)
    {
        const auto bag_array = GW::Items::GetBagArray();
        if (bag_array)
        {
            std::vector<flatbuffers::Offset<GWIPC::Bag>> bags_vector;
            std::vector<flatbuffers::Offset<GWIPC::BagItem>> bag_items_vector;
            for (uint32_t bag_index = 1; bag_index <= 5; bag_index++)
            {
                const auto bag_size = create_bag_item(builder, bag_array, bag_index, bag_items_vector);

                auto bag_items = builder.CreateVector(bag_items_vector);
                GWIPC::BagBuilder bag_builder(builder);
                bag_builder.add_items(bag_items);
                bag_builder.add_bag_size(static_cast<uint8_t>(bag_size));

                auto new_bag = bag_builder.Finish();
                bags_vector.emplace_back(new_bag);
            }
            bags = builder.CreateVector(bags_vector);
        }
    }

    void build_equipped_items(
      flatbuffers::FlatBufferBuilder& builder,
      flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<GWIPC::BagItem>>>& bag_items)
    {
        const auto bag_array = GW::Items::GetBagArray();
        if (bag_array)
        {
            constexpr uint32_t equipment_bag_index = 22;
            std::vector<flatbuffers::Offset<GWIPC::BagItem>> bag_items_vector;
            const auto bag_size = create_bag_item(builder, bag_array, equipment_bag_index, bag_items_vector);

            bag_items = builder.CreateVector(bag_items_vector);
        }
    }

    void build_dialogs_info(flatbuffers::FlatBufferBuilder& builder,
                            flatbuffers::Offset<GWIPC::DialogsInfo> dialogs_info,
                            DialogsManager& dialogs_manager)
    {
        const auto& dialog_messages = dialogs_manager.GetDialogButtonMessages();
        const auto& dialog_button_infos = dialogs_manager.GetDialogButtons();

        const auto dialog_curr_agent_id = dialogs_manager.GetDialogAgentId();
        const auto dialog_last_agent_id = dialogs_manager.last_agent_id;

        const auto last_dialog_id = GW::Agents::GetLastDialogId();

        auto dialog_body_message_offset = builder.CreateString(dialogs_manager.dialog_body.string());

        std::vector<flatbuffers::Offset<GWIPC::GWDialog>> gw_dialogs_vector;
        for (size_t i = 0; i < dialog_messages.size(); i++)
        {
            const auto& dialog_message = dialog_messages[i];
            const auto& dialog_button = dialog_button_infos[i];

            auto dialog_message_offset = builder.CreateString(dialog_message->string());

            GWIPC::GWDialogBuilder gw_dialog_builder(builder);
            gw_dialog_builder.add_id(dialog_button->dialog_id);
            gw_dialog_builder.add_button_icon_id(dialog_button->button_icon);
            gw_dialog_builder.add_skill_id(dialog_button->skill_id);
            gw_dialog_builder.add_text(dialog_message_offset);

            auto new_gw_dialog = gw_dialog_builder.Finish();
            gw_dialogs_vector.emplace_back(new_gw_dialog);
        }

        auto gw_dialogs = builder.CreateVector(gw_dialogs_vector);

        dialogs_info = GWIPC::CreateDialogsInfo(builder, gw_dialogs, last_dialog_id, dialog_curr_agent_id,
                                                dialog_last_agent_id);
    }

    flatbuffers::Offset<GWIPC::BagItem> create_bag_item_from_values(
      flatbuffers::FlatBufferBuilder& builder_, const std::string& description, const std::string& full_name,
      const std::string& single_item_name, const std::string& name, int interaction, bool is_weapon_set_item,
      int item_id, int item_modifier, int model_id, int quantity, int type, int value, int index)
    {
        auto description_offset = builder_.CreateString(description);
        auto full_name_offset = builder_.CreateString(full_name);
        auto single_item_name_offset = builder_.CreateString(single_item_name);
        auto name_offset = builder_.CreateString(name);

        GWIPC::BagItemBuilder bag_item_builder(builder_);
        bag_item_builder.add_description(description_offset);
        bag_item_builder.add_full_name(full_name_offset);
        bag_item_builder.add_name(name_offset);
        bag_item_builder.add_single_item_name(single_item_name_offset);

        bag_item_builder.add_interaction(interaction);
        bag_item_builder.add_is_weapon_set_item(is_weapon_set_item);
        bag_item_builder.add_item_id(item_id);
        bag_item_builder.add_item_modifier(item_modifier);
        bag_item_builder.add_model_id(model_id);
        bag_item_builder.add_quantity(quantity);
        bag_item_builder.add_type(type);
        bag_item_builder.add_value(value);
        bag_item_builder.add_index(index);

        return bag_item_builder.Finish();
    }

    flatbuffers::Offset<GWIPC::Quest>
    create_quest_from_values(flatbuffers::FlatBufferBuilder& builder, const std::string& description,
                             const std::string& location, const std::string& name,
                             const std::string& npc_name, const std::string& objectives, int32_t log_state,
                             int32_t map_from, int32_t map_to, GWIPC::Vec3 marker, int32_t quest_id)
    {
        auto description_offset = builder.CreateString(description);
        auto location_offset = builder.CreateString(location);
        auto name_offset = builder.CreateString(name);
        auto npc_name_offset = builder.CreateString(npc_name);
        auto objectives_offset = builder.CreateString(objectives);

        GWIPC::QuestBuilder quest_builder(builder);
        quest_builder.add_description(description_offset);
        quest_builder.add_location(location_offset);
        quest_builder.add_log_state(log_state);
        quest_builder.add_map_from(map_from);
        quest_builder.add_map_to(map_to);
        quest_builder.add_marker(&marker);
        quest_builder.add_name(name_offset);
        quest_builder.add_npc_name(npc_name_offset);
        quest_builder.add_objectives(objectives_offset);
        quest_builder.add_quest_id(quest_id);

        return quest_builder.Finish();
    }
};
