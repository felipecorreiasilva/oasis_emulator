#include "mapclif.hpp"
#include "../common/packets.hpp"
#include <iostream>

int parse_map(int fd, SessionData& session) {
    if (session.rdata.size() < 2) {
        return 0;
    }

    uint16_t packet_id = *reinterpret_cast<uint16_t*>(session.rdata.data());

    switch (packet_id) {
    case COMMON_PING: {
        std::cout << "[OasisMap] Recebido PING do FD: " << fd << std::endl;
        uint16_t response = COMMON_PING;
        session_write(session, &response, sizeof(response));
        return 2;
    }

    case MAP_ENTER_REQUEST: {
        if (session.rdata.size() < sizeof(PACKET_MAP_ENTER_REQUEST)) {
            return 0;
        }

        PACKET_MAP_ENTER_REQUEST* req = reinterpret_cast<PACKET_MAP_ENTER_REQUEST*>(session.rdata.data());
        std::cout << "[OasisMap] Entrada no mapa solicitada: char_id=" << req->char_id << " map_id=" << req->map_id << " x=" << req->x << " y=" << req->y << std::endl;

        PACKET_MAP_ENTER_ACCEPTED response{};
        response.packet_id = MAP_ENTER_ACCEPTED;
        response.status = 1;
        response.map_id = req->map_id;
        response.x = req->x;
        response.y = req->y;

        session_write(session, &response, sizeof(response));
        return sizeof(PACKET_MAP_ENTER_REQUEST);
    }

    default:
        std::cerr << "[OasisMap] Pacote desconhecido (" << packet_id << ") de FD: " << fd << "." << std::endl;
        return -1;
    }
}
