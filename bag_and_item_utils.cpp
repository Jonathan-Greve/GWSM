#include "pch.h"
#include "bag_and_item_utils.h"

GW::ItemModifier* get_modifier(const uint32_t identifier, const GW::Item* item)
{
    for (size_t i = 0; i < item->mod_struct_size; i++)
    {
        GW::ItemModifier* mod = &item->mod_struct[i];
        if (mod->identifier() == identifier)
            return mod;
    }
    return nullptr;
}

uint32_t get_uses(const uint32_t quantity, const GW::Item* item)
{
    GW::ItemModifier* mod = get_modifier(0x2458, item);
    return mod ? mod->arg2() : quantity;
}

bool is_identification_kit(const GW::Item* item)
{
    GW::ItemModifier* mod = get_modifier(0x25E8, item);
    return mod && mod->arg1() == 1;
}

bool is_tome(const GW::Item* item)
{
    GW::ItemModifier* mod = get_modifier(0x2788, item);
    uint32_t use_id = mod ? mod->arg2() : 0;
    return use_id > 15 && use_id < 36;
}

bool is_lesser_kit(const GW::Item* item)
{
    GW::ItemModifier* mod = get_modifier(0x25E8, item);
    return mod && mod->arg1() == 3;
}

bool is_expert_salvage_kit(const GW::Item* item)
{
    GW::ItemModifier* mod = get_modifier(0x25E8, item);
    return mod && mod->arg1() == 2;
}

bool is_perfect_salvage_kit(const GW::Item* item)
{
    GW::ItemModifier* mod = get_modifier(0x25E8, item);
    return mod && mod->arg1() == 6;
}

bool is_salvage_kit(const GW::Item* item)
{
    return is_lesser_kit(item) || is_expert_salvage_kit(item) || is_perfect_salvage_kit(item);
}

bool is_weapon(const GW::Item* item)
{
    switch (static_cast<GW::Constants::ItemType>(item->type))
    {
    case GW::Constants::ItemType::Axe:
    case GW::Constants::ItemType::Sword:
    case GW::Constants::ItemType::Shield:
    case GW::Constants::ItemType::Scythe:
    case GW::Constants::ItemType::Bow:
    case GW::Constants::ItemType::Wand:
    case GW::Constants::ItemType::Staff:
    case GW::Constants::ItemType::Offhand:
    case GW::Constants::ItemType::Daggers:
    case GW::Constants::ItemType::Hammer:
    case GW::Constants::ItemType::Spear:
        return true;
    default:
        return false;
    }
}

bool is_armor(const GW::Item* item)
{
    switch (static_cast<GW::Constants::ItemType>(item->type))
    {
    case GW::Constants::ItemType::Headpiece:
    case GW::Constants::ItemType::Chestpiece:
    case GW::Constants::ItemType::Leggings:
    case GW::Constants::ItemType::Boots:
    case GW::Constants::ItemType::Gloves:
        return true;
    default:
        return false;
    }
}

bool is_rare_material(const GW::Item* item)
{
    GW::ItemModifier* mod = get_modifier(0x2508, item);
    return mod && mod->arg1() > 11;
}

bool is_weapon_set_item(const GW::Item* item)
{
    if (! is_weapon(item))
        return false;
    GW::GameContext* g = GW::GetGameContext();
    if (! g || ! g->items || ! g->items->inventory)
        return false;
    GW::WeaponSet* weapon_sets = g->items->inventory->weapon_sets;
    for (size_t i = 0; i < 4; i++)
    {
        if (weapon_sets[i].offhand == item)
            return true;
        if (weapon_sets[i].weapon == item)
            return true;
    }
    return false;
}

const GW::Array<GW::TradeContext::Item>* get_player_trade_items()
{
    if (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Outpost)
        return nullptr;
    auto game_context = GW::GetGameContext();
    if (! game_context)
        return nullptr;
    const GW::TradeContext* c = game_context->trade;
    if (! c || ! c->GetIsTradeInitiated())
        return nullptr;
    return &c->player.items;
}

bool is_offered_in_trade(const GW::Item* item)
{
    auto* player_items = get_player_trade_items();
    if (! player_items)
        return false;
    for (auto& player_item : *player_items)
    {
        if (player_item.item_id == item->item_id)
            return true;
    }
    return false;
}

bool is_trade_window_open()
{
    if (GW::Map::GetInstanceType() != GW::Constants::InstanceType::Outpost)
        return false;
    const GW::TradeContext* c = GW::GetGameContext()->trade;
    return c && c->GetIsTradeInitiated();
}

bool is_sparkly(const GW::Item* item) { return (item->interaction & 0x2000) == 0; }

bool is_identified(const GW::Item* item) { return (item->interaction & 1) != 0; }

bool is_stackable(const GW::Item* item) { return (item->interaction & 0x80000) != 0; }

bool is_usable(const GW::Item* item) { return (item->interaction & 0x1000000) != 0; }

bool is_tradable(const GW::Item* item) { return (item->interaction & 0x100) == 0; }

bool is_blue(const GW::Item* item) { return item->single_item_name && item->single_item_name[0] == 0xA3F; }

bool is_purple(const GW::Item* item) { return (item->interaction & 0x400000) != 0; }

bool is_green(const GW::Item* item) { return (item->interaction & 0x10) != 0; }

bool is_gold(const GW::Item* item) { return (item->interaction & 0x20000) != 0; }

bool can_offer_to_trade(const GW::Item* item)
{
    auto* player_items = get_player_trade_items();
    if (! player_items)
        return false;
    return is_tradable(item) && is_trade_window_open() && ! is_offered_in_trade(item) &&
      reinterpret_cast<int>(&player_items) > 0x00000404 && player_items->size() < 7;
}

GW::Constants::Rarity get_rarity(const GW::Item* item)
{
    if (is_green(item))
        return GW::Constants::Rarity::Green;
    if (is_gold(item))
        return GW::Constants::Rarity::Gold;
    if (is_purple(item))
        return GW::Constants::Rarity::Purple;
    if (is_blue(item))
        return GW::Constants::Rarity::Blue;
    return GW::Constants::Rarity::White;
}

bool is_salvagable(const GW::Item* item, const GW::Bag* bag)
{
    if (is_tradable(item) || is_green(item))
        return false; // Non-salvagable flag set
    if (! bag)
        return false;
    if (! bag->IsInventoryBag() && ! bag->IsStorageBag())
        return false;
    if (bag->index + 1 == static_cast<uint32_t>(GW::Constants::Bag::Equipment_Pack))
        return false;
    switch (static_cast<GW::Constants::ItemType>(item->type))
    {
    case GW::Constants::ItemType::Trophy:
        return get_rarity(item) == GW::Constants::Rarity::White && item->info_string &&
          item->is_material_salvageable;
    case GW::Constants::ItemType::Salvage:
    case GW::Constants::ItemType::CC_Shards:
        return true;
    case GW::Constants::ItemType::Materials_Zcoins:
        switch (item->model_id)
        {
        case GW::Constants::ItemID::BoltofDamask:
        case GW::Constants::ItemID::BoltofLinen:
        case GW::Constants::ItemID::BoltofSilk:
        case GW::Constants::ItemID::DeldrimorSteelIngot:
        case GW::Constants::ItemID::ElonianLeatherSquare:
        case GW::Constants::ItemID::LeatherSquare:
        case GW::Constants::ItemID::LumpofCharcoal:
        case GW::Constants::ItemID::RollofParchment:
        case GW::Constants::ItemID::RollofVellum:
        case GW::Constants::ItemID::SpiritwoodPlank:
        case GW::Constants::ItemID::SteelIngot:
        case GW::Constants::ItemID::TemperedGlassVial:
        case GW::Constants::ItemID::VialofInk:
            return true;
        default:
            break;
        }
    default:
        break;
    }
    if (is_weapon(item) || is_armor(item))
        return true;
    return false;
}
