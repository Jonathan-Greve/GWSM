#pragma once
#include "Packets/StoC.h"

struct InstanceLoadData
{
    GW::Packet::StoC::InstanceLoadFile curr_instance_load_file;
};

class InstanceLoadCallbacks
{
public:
    InstanceLoadCallbacks() = default;
    bool init()
    {
        bool success = true;

        success = success &&
          GW::StoC::RegisterPacketCallback<GW::Packet::StoC::InstanceLoadFile>(
                    &InstanceLoadFile_HookEntry,
                    [this](GW::HookStatus* status, GW::Packet::StoC::InstanceLoadFile* packet)
                    { return on_instance_load_file(status, packet); });

        return success;
    }

    void terminate()
    {
        GW::StoC::RemoveCallback<GW::Packet::StoC::InstanceLoadFile>(&InstanceLoadFile_HookEntry);
    }

    std::atomic<bool> instance_load_file_changed = false;

    InstanceLoadData instance_load_data;

private:
    GW::HookEntry InstanceLoadFile_HookEntry;

    void on_instance_load_file(GW::HookStatus* status, GW::Packet::StoC::InstanceLoadFile* packet);
};
