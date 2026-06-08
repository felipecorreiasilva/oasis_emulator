#include "charclif.hpp"
#include "../common/packets.hpp"
#include "../common/sql.hpp"
#include <iostream>
#include <cstring>
#include <vector>
#include <string>

extern Sql db_handle;

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
        session.account_id = req->user_id;
        std::cout << "[OasisChar] Requisicao de charlist para user_id: " << req->user_id << std::endl;

        std::string query = "SELECT char_id, name, base_level, map_id, x, y FROM `char` WHERE account_id = " + std::to_string(req->user_id) + " ORDER BY char_id ASC LIMIT 16";
        if (!db_handle.query_select(query)) {
            std::cerr << "[OasisChar] Erro ao buscar charlist no banco de dados." << std::endl;
            return -1;
        }

        std::vector<p_ac_charlist_entry> entries;
        MYSQL_ROW row = nullptr;
        while ((row = db_handle.fetch_row())) {
            p_ac_charlist_entry entry{};
            entry.char_id = static_cast<uint32_t>(std::stoul(row[0] ? row[0] : "0"));
            strcpy_s(entry.name, sizeof(entry.name), row[1] ? row[1] : "");
            entry.level = static_cast<uint8_t>(std::stoi(row[2] ? row[2] : "1"));
            entry.map_id = static_cast<uint16_t>(std::stoi(row[3] ? row[3] : "1"));
            entry.x = std::stof(row[4] ? row[4] : "150.0");
            entry.y = std::stof(row[5] ? row[5] : "120.0");
            entries.push_back(entry);
        }

        p_ac_charlist_header header{};
        header.packet_id = HEADER_AC_CHARLIST;
        header.count = static_cast<uint8_t>(entries.size());
        session_write(session, &header, sizeof(header));

        for (const auto& entry : entries) {
            session_write(session, &entry, sizeof(entry));
        }

        return sizeof(p_ca_charlist);
    }

    case HEADER_CH_SELECT_CHAR: {
        if (session.rdata.size() < sizeof(p_ch_select_char)) {
            return 0;
        }

        p_ch_select_char* req = reinterpret_cast<p_ch_select_char*>(session.rdata.data());
        std::cout << "[OasisChar] Slot de personagem selecionado: " << static_cast<int>(req->slot) << std::endl;

        if (session.account_id == 0) {
            std::cerr << "[OasisChar] Nenhuma conta autenticada associada a esta sessao." << std::endl;
            return sizeof(p_ch_select_char);
        }

        std::string query = "SELECT char_id, map_id, x, y FROM `char` WHERE account_id = " + std::to_string(session.account_id) + " ORDER BY char_id ASC LIMIT 1 OFFSET " + std::to_string(req->slot);
        if (!db_handle.query_select(query)) {
            std::cerr << "[OasisChar] Erro ao buscar personagem selecionado no banco." << std::endl;
            return sizeof(p_ch_select_char);
        }

        MYSQL_ROW row = db_handle.fetch_row();
        if (!row) {
            std::cerr << "[OasisChar] Slot de personagem invalido para account_id: " << session.account_id << std::endl;
            return sizeof(p_ch_select_char);
        }

        uint32_t char_id = static_cast<uint32_t>(std::stoul(row[0] ? row[0] : "0"));
        uint16_t map_id = static_cast<uint16_t>(std::stoi(row[1] ? row[1] : "1"));
        float x = std::stof(row[2] ? row[2] : "150.0");
        float y = std::stof(row[3] ? row[3] : "120.0");

        p_hc_notify_zonesvr response{};
        response.packet_id = HEADER_HC_NOTIFY_ZONESVR;
        response.char_id = char_id;
        strcpy_s(response.mapname, sizeof(response.mapname), "prt_fild01");
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
