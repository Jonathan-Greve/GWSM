#pragma once
class ItemCallbacks
{
public:
    ItemCallbacks() = default;
    bool init()
    {
        bool success = true;
        success = success &&
          GW::StoC::RegisterPacketCallback(&Items_HookEntry, GAME_SMSG_TRADE_ADD_ITEM,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_trade_add_item(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::WindowItems>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::WindowItems* packet)
                    { return on_window_add_items(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::WindowItemsEnd>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::WindowItemsEnd* packet)
                    { return on_window_items_end(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::ItemStreamEnd>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::ItemStreamEnd* packet)
                    { return on_window_item_stream_end(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::QuotedItemPrice>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::QuotedItemPrice* packet)
                    { return on_item_price_quote(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::WindowPrices>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::WindowPrices* packet)
                    { return on_item_prices(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::ItemUpdateOwner>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::ItemUpdateOwner* packet)
                    { return on_item_update_owner(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback(&Items_HookEntry, GAME_SMSG_ITEM_UPDATE_QUANTITY,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_item_update_quantity(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::ItemCustomisedForPlayer>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::ItemCustomisedForPlayer* packet)
                    { return on_item_update_name(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback(&Items_HookEntry, GAME_SMSG_ITEM_MOVED_TO_LOCATION,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_item_moved_to_location(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback(&Items_HookEntry, GAME_SMSG_INVENTORY_CREATE_BAG,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_inventory_create_bag(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback(&Items_HookEntry, GAME_SMSG_ITEM_WEAPON_SET,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_item_weapon_set(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback(&Items_HookEntry, GAME_SMSG_ITEM_SET_ACTIVE_WEAPON_SET,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_item_set_active_weapon_set(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback(&Items_HookEntry, GAME_SMSG_ITEM_CHANGE_LOCATION,
                                           [this](GW::HookStatus* status, void* packet)
                                           { return on_item_change_location(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::SalvageConsumeItem>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::SalvageConsumeItem* packet)
                    { return on_item_remove(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::ItemGeneral>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::ItemGeneral* packet)
                    { return on_item_general_info(status, packet); });

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::ItemGeneral_ReuseID>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::ItemGeneral_ReuseID* packet)
                    { return on_item_reuse_id(status, packet); });
        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::SalvageSessionDone>(
                    &Items_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::SalvageSessionDone* packet)
                    { return on_item_salvage_session_done(status, packet); });

        GW::StoC::RegisterPacketCallback<GW::Packet::StoC::TransactionDone>(
          &Items_HookEntry,
          [this](GW::HookStatus* status, GW::Packet::StoC::TransactionDone* packet)
          { return on_transaction_done(status, packet); });

        return success;
    }

    bool inventory_or_equipment_changed = false;

private:
    GW::HookEntry Items_HookEntry;

    void on_trade_add_item(GW::HookStatus* status, void* packet);
    void on_window_add_items(GW::HookStatus* status, GW::Packet::StoC::WindowItems* packet);
    void on_window_items_end(GW::HookStatus* status, GW::Packet::StoC::WindowItemsEnd* packet);
    void on_window_item_stream_end(GW::HookStatus* status, GW::Packet::StoC::ItemStreamEnd* packet);
    void on_item_price_quote(GW::HookStatus* status, GW::Packet::StoC::QuotedItemPrice* packet);
    void on_item_prices(GW::HookStatus* status, GW::Packet::StoC::WindowPrices* packet);
    void on_item_update_owner(GW::HookStatus* status, GW::Packet::StoC::ItemUpdateOwner* packet);
    void on_item_update_quantity(GW::HookStatus* status, void* packet);
    void on_item_update_name(GW::HookStatus* status, GW::Packet::StoC::ItemCustomisedForPlayer* packet);
    void on_item_moved_to_location(GW::HookStatus* status, void* packet);
    void on_inventory_create_bag(GW::HookStatus* status, void* packet);
    void on_item_weapon_set(GW::HookStatus* status, void* packet);
    void on_item_set_active_weapon_set(GW::HookStatus* status, void* packet);
    void on_item_change_location(GW::HookStatus* status, void* packet);
    void on_item_remove(GW::HookStatus* status, GW::Packet::StoC::SalvageConsumeItem* packet);
    void on_item_general_info(GW::HookStatus* status, GW::Packet::StoC::ItemGeneral* packet);
    void on_item_reuse_id(GW::HookStatus* status, GW::Packet::StoC::ItemGeneral_ReuseID* packet);
    void on_item_salvage_session_done(GW::HookStatus* status, GW::Packet::StoC::SalvageSessionDone* packet);
    void on_transaction_done(GW::HookStatus* status, GW::Packet::StoC::TransactionDone* packet);
};
