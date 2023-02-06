#include "pch.h"
#include "UpdateNavMesh.h"

std::map<uint32_t, std::string> existing_nav_mesh_file_paths;

std::string empty_str;

const std::string& update_nav_mesh()
{
    const GW::CharContext* const char_context = GW::GetCharContext();
    if (char_context)
    {
        const auto map_id = char_context->map_id;

        if (! existing_nav_mesh_file_paths.contains(map_id))
        {
            auto file_dir = GetPathToProgramDirectory();
            if (file_dir.has_value())
            {

                const auto file_path = std::format("{}\\nav_mesh_{}.txt", file_dir.value(), map_id);
                if (! std::filesystem::exists(file_path))
                {

                    HANDLE mutex = CreateMutex(NULL, false, L"NavMeshMutex");
                    if (mutex != NULL)
                    {

                        DWORD wait_result = WaitForSingleObject(mutex, 0);
                        if (wait_result == WAIT_OBJECT_0)
                        {
                            if (! std::filesystem::exists(file_dir.value()))
                            {
                                if (! std::filesystem::create_directories(file_dir.value()))
                                {
                                    throw "Could not create program directory.";
                                };
                            }

                            flatbuffers::FlatBufferBuilder builder;
                            flatbuffers::Offset<GWIPC::NavMesh> nav_mesh;
                            make_nav_mesh(builder, nav_mesh);

                            builder.Finish(nav_mesh);

                            const auto size = builder.GetSize();
                            const auto buffer_ptr = builder.GetBufferPointer();
                            const auto buffer = std::vector<uint8_t>(buffer_ptr, buffer_ptr + size);

                            std::ofstream navmesh_ofstream(file_path,
                                                           std::ios_base::binary | std::ios_base::out);

                            if (navmesh_ofstream.is_open())
                            {
                                navmesh_ofstream.write(reinterpret_cast<const char*>(buffer.data()),
                                                       buffer.size());
                                navmesh_ofstream.close();
                            }

                            ReleaseMutex(mutex);
                        }

                        CloseHandle(mutex);
                    }
                }
                else
                {
                    existing_nav_mesh_file_paths.insert({map_id, file_path});
                }
            }
        }
        else
        {
            return existing_nav_mesh_file_paths[map_id];
        }
    }

    return empty_str;
}

void make_nav_mesh(flatbuffers::FlatBufferBuilder& builder_, flatbuffers::Offset<GWIPC::NavMesh>& nav_mesh)
{
    const GW::GameContext* const game_context = GW::GetGameContext();
    if (! game_context)
        return;

    const GW::MapContext* const map_context = game_context->map;
    if (! map_context)
        return;

    const GW::Array<uint32_t>& sub1s = map_context->sub1->pathing_map_block;
    const auto path_map = GW::Map::GetPathingMap();

    auto min_x = std::numeric_limits<float>::max();
    auto max_x = std::numeric_limits<float>::min();
    auto min_y = std::numeric_limits<float>::max();
    auto max_y = std::numeric_limits<float>::min();
    auto min_z = std::numeric_limits<float>::max();
    auto max_z = std::numeric_limits<float>::min();

    std::vector<flatbuffers::Offset<GWIPC::NavMeshTrapezoid>> nav_mesh_trapezoids_vector;
    for (size_t i = 0; i < path_map->size(); ++i)
    {
        const GW::PathingMap pmap = path_map->m_buffer[i];
        for (size_t j = 0; j < pmap.trapezoid_count; ++j)
        {
            GW::PathingTrapezoid& trapezoid = pmap.trapezoids[j];
            bool is_traversable = ! sub1s[i];

            float radius = 10;

            // Get height of vertices and convert to a LH coordinate system.
            float altitude = 0;
            GW::GamePos gp{trapezoid.XBL, trapezoid.YB, i};
            GW::Map::QueryAltitude(gp, radius, altitude);
            GWIPC::Vec3 BL{trapezoid.XBL, -altitude, trapezoid.YB};

            GW::GamePos gp2{trapezoid.XBR, trapezoid.YB, i};
            GW::Map::QueryAltitude(gp2, radius, altitude);
            GWIPC::Vec3 BR{trapezoid.XBR, -altitude, trapezoid.YB};

            GW::GamePos gp3{trapezoid.XTL, trapezoid.YT, i};
            GW::Map::QueryAltitude(gp3, radius, altitude);
            GWIPC::Vec3 TL{trapezoid.XTL, -altitude, trapezoid.YT};

            GW::GamePos gp4{trapezoid.XTR, trapezoid.YT, i};
            GW::Map::QueryAltitude(gp4, radius, altitude);
            GWIPC::Vec3 TR{trapezoid.XTR, -altitude, trapezoid.YT};

            std::vector<GWIPC::Vec3> trapezoid_vertices = {BL, BR, TL, TR};
            min_x = min_element(trapezoid_vertices.begin(), trapezoid_vertices.end(),
                                [](const GWIPC::Vec3& a, const GWIPC::Vec3& b) { return a.x() < b.x(); })
                      ->x();
            max_x = max_element(trapezoid_vertices.begin(), trapezoid_vertices.end(),
                                [](const GWIPC::Vec3& a, const GWIPC::Vec3& b) { return a.x() < b.x(); })
                      ->x();
            min_y = min_element(trapezoid_vertices.begin(), trapezoid_vertices.end(),
                                [](const GWIPC::Vec3& a, const GWIPC::Vec3& b) { return a.y() < b.y(); })
                      ->y();
            max_y = max_element(trapezoid_vertices.begin(), trapezoid_vertices.end(),
                                [](const GWIPC::Vec3& a, const GWIPC::Vec3& b) { return a.y() < b.y(); })
                      ->y();
            min_z = min_element(trapezoid_vertices.begin(), trapezoid_vertices.end(),
                                [](const GWIPC::Vec3& a, const GWIPC::Vec3& b) { return a.z() < b.z(); })
                      ->z();
            max_z = max_element(trapezoid_vertices.begin(), trapezoid_vertices.end(),
                                [](const GWIPC::Vec3& a, const GWIPC::Vec3& b) { return a.z() < b.z(); })
                      ->z();

            int32_t left_id = -1;
            int32_t right_id = -1;
            int32_t down_id = -1;
            int32_t up_id = -1;
            if (trapezoid.adjacent[0])
                left_id = trapezoid.adjacent[0]->id;
            if (trapezoid.adjacent[1])
                right_id = trapezoid.adjacent[1]->id;
            if (trapezoid.adjacent[2])
                down_id = trapezoid.adjacent[2]->id;
            if (trapezoid.adjacent[3])
                up_id = trapezoid.adjacent[3]->id;

            GWIPC::AdjacentTrapezoidIds adjacent_trapezoid_ids{left_id, right_id, down_id, up_id};

            auto trapezoid_builder = GWIPC::NavMeshTrapezoidBuilder(builder_);
            trapezoid_builder.add_id(trapezoid.id);
            trapezoid_builder.add_adjacent_trapezoid_ids(&adjacent_trapezoid_ids);
            trapezoid_builder.add_bottom_left(&BL);
            trapezoid_builder.add_bottom_right(&BR);
            trapezoid_builder.add_top_left(&TL);
            trapezoid_builder.add_top_right(&TR);
            trapezoid_builder.add_z_plane(pmap.zplane);

            auto new_trapezoid = trapezoid_builder.Finish();

            nav_mesh_trapezoids_vector.push_back(new_trapezoid);
        }
    }

    GWIPC::NavMeshDimensions nav_mesh_dimensions(min_x, min_y, min_z, max_x, max_y, max_z);

    auto trapezoids = builder_.CreateVector(nav_mesh_trapezoids_vector);

    auto nav_mesh_builder = GWIPC::NavMeshBuilder(builder_);
    nav_mesh_builder.add_trapezoids(trapezoids);
    nav_mesh_builder.add_dimensions(&nav_mesh_dimensions);

    nav_mesh = nav_mesh_builder.Finish();
}
