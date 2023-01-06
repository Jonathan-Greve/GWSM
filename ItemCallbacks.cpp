#include "pch.h"
#include "ItemCallbacks.h"

void ItemCallbacks::on_trade_add_item(GW::HookStatus* status, void* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_window_add_items(GW::HookStatus* status, GW::Packet::StoC::WindowItems* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_window_items_end(GW::HookStatus* status, GW::Packet::StoC::WindowItemsEnd* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_window_item_stream_end(GW::HookStatus* status, GW::Packet::StoC::ItemStreamEnd* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_item_price_quote(GW::HookStatus* status, GW::Packet::StoC::QuotedItemPrice* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_item_prices(GW::HookStatus* status, GW::Packet::StoC::WindowPrices* packet)
{
    inventory_or_equipment_changed = true;
}

// Called when item is dropped by monsterod death.
void ItemCallbacks::on_item_update_owner(GW::HookStatus* status, GW::Packet::StoC::ItemUpdateOwner* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_item_update_quantity(GW::HookStatus* status, void* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_item_update_name(GW::HookStatus* status,
                                        GW::Packet::StoC::ItemCustomisedForPlayer* packet)
{
    inventory_or_equipment_changed = true;
}

// Called when picking up an item or buying an item, or getting salvage stuff.
void ItemCallbacks::on_item_moved_to_location(GW::HookStatus* status, void* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_inventory_create_bag(GW::HookStatus* status, void* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_item_weapon_set(GW::HookStatus* status, void* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_item_set_active_weapon_set(GW::HookStatus* status, void* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_item_change_location(GW::HookStatus* status, void* packet)
{
    inventory_or_equipment_changed = true;
}

// Also called when dropping item, or putting it in the trash bin or selling it, or destroyed in salvage.
void ItemCallbacks::on_item_remove(GW::HookStatus* status, GW::Packet::StoC::SalvageConsumeItem* packet)
{
    inventory_or_equipment_changed = true;
}

// Called when identifying an item, or buying an item, salvaging an item.
void ItemCallbacks::on_item_general_info(GW::HookStatus* status, GW::Packet::StoC::ItemGeneral* packet)
{
    inventory_or_equipment_changed = true;
}

void ItemCallbacks::on_item_reuse_id(GW::HookStatus* status, GW::Packet::StoC::ItemGeneral_ReuseID* packet) {
}

// When salvage is done.
void ItemCallbacks::on_item_salvage_session_done(GW::HookStatus* status,
                                                 GW::Packet::StoC::SalvageSessionDone* packet)
{
    inventory_or_equipment_changed = true;
}

//called when buying an item or selling it.
void ItemCallbacks::on_transaction_done(GW::HookStatus* status, GW::Packet::StoC::TransactionDone* packet)
{
    inventory_or_equipment_changed = true;
}
