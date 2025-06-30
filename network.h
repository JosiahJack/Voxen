#ifndef VOXEN_NETWORK_H
#define VOXEN_NETWORK_H

typedef enum {
    MODE_LISTEN_SERVER,    // Runs both server and client locally
    //MODE_DEDICATED_SERVER, // Server only, no rendering (headless) Currently only using Listen for coop
    MODE_CLIENT            // Client only, connects to a server
} EngineMode;

extern EngineMode engine_mode;
extern char* server_address;
extern int server_port;

int InitializeNetworking(void);
void CleanupNetworking(void);

#endif // VOXEN_NETWORK_H
