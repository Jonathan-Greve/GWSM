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

    void update()
    {
        // Create a flatbuffer builder object
        flatbuffers::FlatBufferBuilder builder;

        // Create the Character object
        flatbuffers::Offset<GWIPC::Character> character;
        build_character(builder, character);

        // Create the Instance object
        auto instance = GWIPC::CreateInstance(builder, 26, 27);

        auto party = GWIPC::CreateParty(builder, 123, 42);

        // Create the ClientData object
        auto client_data = GWIPC::CreateClientData(builder, character, instance, party);

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
        // Create the vectors for the nested Vec2 and Vec3 structs
        auto position = GWIPC::Vec3(1.0f, 2.0f, 3.0f);
        auto terrain_normal = GWIPC::Vec3(4.0f, 5.0f, 6.0f);
        auto velocity = GWIPC::Vec2(7.0f, 8.0f);

        // Create the Agent object
        GWIPC::Agent agent(1, position, terrain_normal, 9.0f, velocity, 10.0f, 11.0f, 12);

        // Create the string for the name field
        auto name = builder.CreateString("Test Character");

        //Create the AgentLiving object
        auto agent_living = CreateAgentLiving(builder, &agent, name, 13, 14.0f, 15.0f, 16.0f, 17,
                                              (GWIPC::Profession)1, (GWIPC::Profession)2, (GWIPC::TeamColor)3,
                                              18, 19, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f, 25.0f);

        character = GWIPC::CreateCharacter(builder, agent_living);
    }
};
