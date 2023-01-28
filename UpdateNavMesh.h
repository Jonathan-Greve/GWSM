#pragma once
struct NavMesh;

const std::string& update_nav_mesh();
void make_nav_mesh(flatbuffers::FlatBufferBuilder& builder_, flatbuffers::Offset<GWIPC::NavMesh>& nav_mesh);
