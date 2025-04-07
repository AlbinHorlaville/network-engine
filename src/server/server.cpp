#include <enet6/enet.h>
#include <iostream>

int main(int argc, char** argv) {
    if (enet_initialize() != 0) {
        std::cerr << "Error during ENet6 initialization." << std::endl;
        return EXIT_FAILURE;
    }

    atexit(enet_deinitialize); // Nettoyage automatique à la fin du programme

    ENetAddress address;
    enet_address_build_any(&address, ENET_ADDRESS_TYPE_IPV6);
    address.port = 5555;

    ENetHost* server;
    server = enet_host_create (ENET_ADDRESS_TYPE_ANY, /* either has to match address->type or be ENET_ADDRESS_TYPE_ANY to dual stack the socket */
                               & address /* the address to bind the server host to */,
                               4      /* allow up to 4     clients and/or outgoing connections */,
                               2      /* allow up to 2 channels to be used, 0 and 1 */,
                               0      /* assume any amount of incoming bandwidth */,
                               0      /* assume any amount of outgoing bandwidth */);

    if (server == nullptr) {
        std::cerr << "Error during Enet6 client creation." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Server listening port : " << address.port << "..." << std::endl;

    ENetEvent event;
    while (true) {
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    char ipStr[46]; // INET6_ADDRSTRLEN
                    enet_address_get_host_ip(&event.peer->address, ipStr, sizeof(ipStr));
                    std::cout << "New client connected on " << ipStr << std::endl;
                    event.peer->data = (void*)"Client connected";
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "Package received on canal " << event.channelID
                              << " : " << (char*)event.packet->data << std::endl;

                    // Réponse (echo)
                    enet_peer_send(event.peer, event.channelID, event.packet);
                    enet_host_flush(server); // Envoi immédiat

                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client disconnected." << std::endl;
                    event.peer->data = nullptr;
                    break;

                default:
                    break;
            }
        }
    }

    enet_host_destroy(server);
    return EXIT_SUCCESS;
}
