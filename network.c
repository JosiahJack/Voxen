#include <stdio.h>
#include <enet/enet.h>
#include "network.h"
#include "debug.h"

EngineMode engine_mode = MODE_LISTEN_SERVER; // Default mode
char* server_address = "127.0.0.1"; // Default to localhost for listen server
int server_port = 27015; // Default port

ENetHost* server_host = NULL;
ENetHost* client_host = NULL;
ENetPeer* server_peer = NULL; // Client's connection to server

int InitializeNetworking(void) {
    if (enet_initialize() != 0) { DualLogError("ENet initialization failed\n"); return -1; }
    
    // Initialize client networking
    ENetAddress address;
    enet_address_set_host(&address, server_address);
    address.port = server_port;
    client_host = enet_host_create(NULL, 1, 2, 0, 0);
    if (!client_host) { DualLogError("Failed to create ENet client host\n"); return -1; }
    
    server_peer = enet_host_connect(client_host, &address, 2, 0);
    if (!server_peer) { DualLogError("Failed to connect to server\n"); return -1; }
    
    return 0;
}

void CleanupNetworking(void) {
    if (client_host) enet_host_destroy(client_host);
    if (server_host) enet_host_destroy(server_host);
    enet_deinitialize();
}
