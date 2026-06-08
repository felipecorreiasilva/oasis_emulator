#include "mapclif.hpp"
#include "../common/packets.hpp"
#include <iostream>

int parse_map(int fd, SessionData& session) {
    if (session.rdata.size() < 2) {
        return 0;
    }

    uint16_t packet_id = *reinterpret_cast<uint16_t*>(session.rdata.data());

    switch (packet_id) {
    case HEADER_PING: {
        std::cout << "[OasisMap] Recebido PING do FD: " << fd << std::endl;
        uint16_t response = HEADER_PING;
        session_write(session, &response, sizeof(response));
        return 2;
    }

    case HEADER_CA_ENTER_MAP: {
        if (session.rdata.size() < sizeof(p_ca_enter_map)) {
            return 0;
        }

        p_ca_enter_map* req = reinterpret_cast<p_ca_enter_map*>(session.rdata.data());
        std::cout << "[OasisMap] Entrada no mapa solicitada: char_id=" << req->char_id << " map_id=" << req->map_id << " x=" << req->x << " y=" << req->y << std::endl;

        p_ac_enter_map response{};
        response.packet_id = HEADER_AC_ENTER_MAP;
        response.status = 1;
        response.map_id = req->map_id;
        response.x = req->x;
        response.y = req->y;

        session_write(session, &response, sizeof(response));
        return sizeof(p_ca_enter_map);
    }

    default:
        std::cerr << "[OasisMap] Pacote desconhecido (" << packet_id << ") de FD: " << fd << "." << std::endl;
        return -1;
    }
}
