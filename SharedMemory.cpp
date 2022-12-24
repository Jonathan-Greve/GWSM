#include "pch.h"
#include "SharedMemory.h"

SharedMemory::~SharedMemory()
{
    shared_memory_object::remove(m_name.c_str());
    auto connected_clients = m_managed_shared_memory.find_or_construct<ConnectedClients>(unique_instance)();
    connected_clients->disconnect(m_name);
}

void SharedMemory::init(std::string shared_memory_name)
{
    m_name = shared_memory_name;

    shared_memory_object::remove(m_name.c_str());
    m_managed_shared_memory = managed_shared_memory(open_or_create, m_name.c_str(), 65536);

    // Construct shared_memory singleton shared by all connected GW clients. This can be used by external
    // processes for finding the shared memory for each client.
    auto connected_clients = m_managed_shared_memory.find_or_construct<ConnectedClients>(unique_instance)();
    connected_clients->connect(m_name);
}
