#include "charclif.hpp"
#include "../common/packets.hpp"
#include "../common/sql.hpp"
#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <cctype>

extern Sql db_handle;

static std::string sql_escape_literal(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size() * 2);

    for (char ch : value) {
        if (ch == '\'' || ch == '\\') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }

    return escaped;
}

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

    // 1. Atualizado: Buscando apenas os campos da sua nova struct
    // Use uma string simples e clara para evitar erros de sintaxe
std::string query = "SELECT char_id, char_num, name, base_level, sex, hair, map_id FROM `char` WHERE account_id = " 
                    + std::to_string(req->user_id) + " ORDER BY char_num ASC LIMIT 16";

    if (!db_handle.query_select(query)) {
        std::cerr << "[OasisChar] Erro ao buscar charlist no banco de dados." << std::endl;
        return -1;
    }

    std::vector<p_ac_charlist_entry> entries;
    MYSQL_ROW row = nullptr;
    while ((row = db_handle.fetch_row())) {
        p_ac_charlist_entry entry{};
        
        // Mapeamento dos campos
        entry.char_id = static_cast<uint32_t>(std::stoul(row[0] ? row[0] : "0"));
        entry.char_num = static_cast<uint8_t>(std::stoi(row[1] ? row[1] : "0"));
        strcpy_s(entry.name, sizeof(entry.name), row[2] ? row[2] : "");
        entry.level = static_cast<uint8_t>(std::stoi(row[3] ? row[3] : "1"));
        
        // Conversão: SQL 'M'/'F' para 1/0
        std::string sex_str = row[4] ? row[4] : "M";
        entry.sex = (sex_str == "M") ? 1 : 0;
        
        entry.hair = static_cast<uint8_t>(std::stoi(row[5] ? row[5] : "0"));
        entry.map_id = static_cast<uint16_t>(std::stoi(row[6] ? row[6] : "1"));
        
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

        std::string query = "SELECT char_id, map_id, last_x, last_y, last_z FROM `char` WHERE account_id = " + 
                    std::to_string(session.account_id) + " AND char_num = " + 
                    std::to_string(req->slot) + " LIMIT 1";

        if (!db_handle.query_select(query)) {
            std::cerr << "[OasisChar] Erro ao buscar personagem no slot " << (int)req->slot << " no banco." << std::endl;
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
        response.port = 5121;

        session_write(session, &response, sizeof(response));
        return sizeof(p_ch_select_char);
    }

    case HEADER_CH_MAKE_CHAR: {
        if (session.rdata.size() < sizeof(p_ch_make_char)) {
            return 0;
        }

        p_ch_make_char* req = reinterpret_cast<p_ch_make_char*>(session.rdata.data());
        std::string name(req->name);
        std::string escaped_name = sql_escape_literal(name);

        std::cout << "[OasisChar] Pedido de criacao de personagem: name=" << name
                  << " slot=" << static_cast<int>(req->slot)
                  << " job=" << req->job
                  << " sex=" << static_cast<int>(req->sex) << std::endl;

        if (session.account_id == 0) {
            std::cerr << "[OasisChar] Nenhuma conta autenticada associada a esta sessao." << std::endl;

            p_hc_refuse_makechar refuse{};
            refuse.packet_id = HEADER_HC_REFUSE_MAKECHAR;
            refuse.error_code = 1;
            session_write(session, &refuse, sizeof(refuse));
            return sizeof(p_ch_make_char);
        }

        if (name.empty()) {
            std::cerr << "[OasisChar] Nome de personagem vazio." << std::endl;

            p_hc_refuse_makechar refuse{};
            refuse.packet_id = HEADER_HC_REFUSE_MAKECHAR;
            refuse.error_code = 2;
            session_write(session, &refuse, sizeof(refuse));
            return sizeof(p_ch_make_char);
        }

        std::string check_query = "SELECT char_id FROM `char` WHERE name = '" + escaped_name + "' LIMIT 1";
        if (!db_handle.query_select(check_query)) {
            std::cerr << "[OasisChar] Erro ao verificar nome do personagem." << std::endl;
            return -1;
        }

        if (db_handle.fetch_row()) {
            std::cerr << "[OasisChar] Nome de personagem ja existe: " << name << std::endl;

            p_hc_refuse_makechar refuse{};
            refuse.packet_id = HEADER_HC_REFUSE_MAKECHAR;
            refuse.error_code = 3;
            session_write(session, &refuse, sizeof(refuse));
            return sizeof(p_ch_make_char);
        }

        if (req->slot >= MAX_CHAR_SLOTS) {
            std::cerr << "[OasisChar] Tentativa de criar em slot inexistente: " << (int)req->slot << std::endl;
            p_hc_refuse_makechar refuse{};
            refuse.packet_id = HEADER_HC_REFUSE_MAKECHAR;
            refuse.error_code = 5; // Defina um código de erro apropriado para "Slot Inválido"
            session_write(session, &refuse, sizeof(refuse));
            return sizeof(p_ch_make_char);
        }

        db_handle.free_result();
        

        std::string insert_query =
            "INSERT INTO `char` (`char_num`, `account_id`, `name`, `base_level`, `job`, `map_id`, `last_x`, `last_y`, `last_z`, `hair`, `sex`, `zeny`, `state`) VALUES (" +
            std::to_string(req->slot) + ", " + 
            std::to_string(session.account_id) + ", '" + 
            escaped_name + "', 1, " + 
            std::to_string(req->job) + ", 1, 150.0, 120.0, 0.0, " +
            std::to_string(req->hair_style) + ", '" + 
            std::string(req->sex == 1 ? "M" : "F") + "', 0, 0)";

        if (!db_handle.query(insert_query)) {
            std::cerr << "[OasisChar] Falha ao inserir novo personagem no banco." << std::endl;

            p_hc_refuse_makechar refuse{};
            refuse.packet_id = HEADER_HC_REFUSE_MAKECHAR;
            refuse.error_code = 4;
            session_write(session, &refuse, sizeof(refuse));
            return sizeof(p_ch_make_char);
        }

        std::string id_query = "SELECT LAST_INSERT_ID()";
        if (!db_handle.query_select(id_query)) {
            std::cerr << "[OasisChar] Falha ao recuperar char_id criado." << std::endl;
            return -1;
        }

        MYSQL_ROW id_row = db_handle.fetch_row();
        uint32_t char_id = id_row ? static_cast<uint32_t>(std::stoul(id_row[0] ? id_row[0] : "0")) : 0;

        p_hc_accept_makechar accept{};
        accept.packet_id = HEADER_HC_ACCEPT_MAKECHAR;
        accept.result = 1;
        accept.char_id = char_id;
        strcpy_s(accept.name, sizeof(accept.name), req->name);

        session_write(session, &accept, sizeof(accept));
        return sizeof(p_ch_make_char);
    }

    default:
        std::cerr << "[OasisChar] Pacote desconhecido (" << packet_id << ") de FD: " << fd << "." << std::endl;
        return -1;
    }
}
