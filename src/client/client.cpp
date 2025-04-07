#include <enet6/enet.h>
#include <iostream>
#include <cstring>  // Pour memcpy

int main() {
    if (enet_initialize() != 0) {
        std::cerr << "Error during ENet6 initialization." << std::endl;
        return EXIT_FAILURE;
    }

    ENetHost* client = enet_host_create(ENET_ADDRESS_TYPE_IPV6, nullptr, 1, 2, 0, 0);
    if (client == nullptr) {
        std::cerr << "Error during Enet6 client creation." << std::endl;
        return EXIT_FAILURE;
    }

    ENetAddress address;
    enet_address_set_host(&address, ENET_ADDRESS_TYPE_IPV6, "::1"); // Adresse du serveur
    address.port = 5555;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == nullptr) {
        std::cerr << "Impossible to connect to the server" << std::endl;
        enet_host_destroy(client);
        return EXIT_FAILURE;
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connected to the server." << std::endl;

        const char* msg = "Hi server !";
        ENetPacket* packet = enet_packet_create(msg, std::strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
    } else {
        std::cerr << "Connection to the server failed." << std::endl;
        enet_peer_reset(peer);
        enet_host_destroy(client);
        return EXIT_FAILURE;
    }

    // Boucle d'écoute des messages entrants
    while (true) {
        while (enet_host_service(client, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "Message received : " << event.packet->data << std::endl;
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Disconnected from the server." << std::endl;
                    return 0;

                default:
                    break;
            }
        }
    }

    enet_host_destroy(client);
    return 0;
}
