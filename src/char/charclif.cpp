#include "charclif.hpp"
#include "../common/packets.hpp"
#include <iostream>
#include <cstring>

int parse_char(int fd, SessionData& session) {
    if (session.rdata.size() < 2) {
        return 0;
    }

    uint16_t packet_id = *reinterpret_cast<uint16_t*>(session.rdata.data());

    switch (packet_id) {
    case HEADER_PING: {
        std::cout << "[OasisChar] Recebido PING do FD: " << fd << std::endl;
        uint16_t response = HEADER_PING;
        session_write(session, &response, sizeof(response));
        return 2;
    }

    case HEADER_CA_CHARLIST: {
        if (session.rdata.size() < sizeof(p_ca_charlist)) {
            return 0;
        }

        p_ca_charlist* req = reinterpret_cast<p_ca_charlist*>(session.rdata.data());
        std::cout << "[OasisChar] Requisicao de charlist para user_id: " << req->user_id << std::endl;

        p_ac_charlist_header header{};
        header.packet_id = HEADER_AC_CHARLIST;
        header.count = 1;

        p_ac_charlist_entry entry{};
        entry.char_id = 1001;
        std::memcpy(entry.name, "OasisHero", sizeof("OasisHero"));
        entry.level = 1;
        entry.map_id = 1;
        entry.x = 150;
        entry.y = 120;

        session_write(session, &header, sizeof(header));
        session_write(session, &entry, sizeof(entry));

        return sizeof(p_ca_charlist);
    }

    case HEADER_CH_SELECT_CHAR: {
        if (session.rdata.size() < sizeof(p_ch_select_char)) {
            return 0;
        }

        p_ch_select_char* req = reinterpret_cast<p_ch_select_char*>(session.rdata.data());
        std::cout << "[OasisChar] Slot de personagem selecionado: " << static_cast<int>(req->slot) << std::endl;

        p_hc_notify_zonesvr response{};
        response.packet_id = HEADER_HC_NOTIFY_ZONESVR;
        response.char_id = 1001;
        std::memcpy(response.mapname, "prt_fild01", sizeof(response.mapname));
        response.ip = 0x0100007F; // 127.0.0.1 little-endian
        response.port = 6902;

        session_write(session, &response, sizeof(response));
        return sizeof(p_ch_select_char);
    }

    default:
        std::cerr << "[OasisChar] Pacote desconhecido (" << packet_id << ") de FD: " << fd << "." << std::endl;
        return -1;
    }
}
