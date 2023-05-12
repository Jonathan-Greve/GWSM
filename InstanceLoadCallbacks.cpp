#include "pch.h"
#include "InstanceLoadCallbacks.h"

void InstanceLoadCallbacks::on_instance_load_file(GW::HookStatus* status,
                                                  GW::Packet::StoC::InstanceLoadFile* packet)
{
    instance_load_file_changed = true;
    instance_load_data.curr_instance_load_file.map_fileID = packet->map_fileID;
    instance_load_data.curr_instance_load_file.spawn_point = packet->spawn_point;
    instance_load_data.curr_instance_load_file.spawn_plane = packet->spawn_plane;
    instance_load_data.curr_instance_load_file.unk1 = packet->unk1;
    instance_load_data.curr_instance_load_file.unk2 = packet->unk2;
    std::memcpy(instance_load_data.curr_instance_load_file.unk3, packet->unk3, 8);
}
