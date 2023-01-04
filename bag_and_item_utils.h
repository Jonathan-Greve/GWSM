#pragma once
GW::ItemModifier* get_modifier(const uint32_t identifier, const GW::Item* item);

uint32_t get_uses(const uint32_t quantity, const GW::Item* item);

bool is_identification_kit(const GW::Item* item);

bool is_tome(const GW::Item* item);

bool is_lesser_kit(const GW::Item* item);

bool is_expert_salvage_kit(const GW::Item* item);

bool is_perfect_salvage_kit(const GW::Item* item);

bool is_salvage_kit(const GW::Item* item);

bool is_weapon(const GW::Item* item);

bool is_armor(const GW::Item* item);

bool is_rare_material(const GW::Item* item);

bool is_weapon_set_item(const GW::Item* item);

const GW::Array<GW::TradeContext::Item>* get_player_trade_items();

bool is_offered_in_trade(const GW::Item* item);

bool is_trade_window_open();

bool is_sparkly(const GW::Item* item);
bool is_identified(const GW::Item* item);
bool is_stackable(const GW::Item* item);
bool is_usable(const GW::Item* item);
bool is_tradable(const GW::Item* item);
bool is_blue(const GW::Item* item);
bool is_purple(const GW::Item* item);
bool is_green(const GW::Item* item);
bool is_gold(const GW::Item* item);

bool can_offer_to_trade(const GW::Item* item);

namespace GW
{
namespace Constants
{
    enum class Rarity : uint8_t
    {
        White,
        Blue,
        Purple,
        Gold,
        Green
    };
}
} // namespace GW

GW::Constants::Rarity get_rarity(const GW::Item* item);

bool is_salvagable(const GW::Item* item, const GW::Bag* bag);
