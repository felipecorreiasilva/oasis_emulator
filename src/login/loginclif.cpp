#include "loginclif.hpp"
#include "../common/packets.hpp"
#include "../common/sql.hpp" 
#include <iostream>
#include <string>

// Puxa a instância global já conectada no login.cpp externa para este escopo
extern Sql db_handle;

int parse_login(int fd, SessionData& session) {
    if (session.rdata.size() < 2) {
        return 0;
    }

    uint16_t packet_id = *reinterpret_cast<uint16_t*>(session.rdata.data());

    switch (packet_id) {
    case COMMON_PING: {
        std::cout << "[OasisLogin] Recebido comando PING do FD: " << fd << std::endl;
        uint16_t response = COMMON_PING;
        session_write(session, &response, sizeof(response));
        return 2;
    }

    case LOGIN_REQUEST: {
        if (session.rdata.size() < sizeof(PACKET_LOGIN_REQUEST)) {
            return 0;
        }

        PACKET_LOGIN_REQUEST* req = reinterpret_cast<PACKET_LOGIN_REQUEST*>(session.rdata.data());

        std::string user(req->username);
        std::string pass(req->password);

        std::cout << "\n[OasisLogin] >>> REQUISICAO DE LOGIN DETECTADA! <<<" << std::endl;
        std::cout << "  -> Usuario Digitado: " << user << std::endl;
        std::cout << "  -> Versao Client:    " << req->client_version << std::endl;
        std::cout << "==================================================" << std::endl;

        std::string query_str = "SELECT account_id, userid, user_pass, sex FROM login WHERE userid = '" + user + "' LIMIT 1";

        if (!db_handle.query_select(query_str)) {
            std::cerr << "[OasisLogin] Erro critico ao executar query no banco de dados." << std::endl;
            return -1;
        }

        MYSQL_ROW row = db_handle.fetch_row();

        if (!row) {
            std::cout << "[OasisLogin] Falha de Autenticacao: Usuario '" << user << "' nao existe." << std::endl;

            PACKET_LOGIN_DENIED refuse{};
            refuse.packet_id = LOGIN_DENIED;
            refuse.error_code = 1; 
            session_write(session, &refuse, sizeof(refuse));

            return sizeof(PACKET_LOGIN_REQUEST);
        }

        uint32_t db_account_id = std::stoul(row[0]);
        std::string db_pass = row[2];
        std::string db_sex = row[3];

        if (pass != db_pass) {
            std::cout << "[OasisLogin] Falha de Autenticacao: Senha incorreta para o usuario '" << user << "'." << std::endl;

            PACKET_LOGIN_DENIED refuse{};
            refuse.packet_id = LOGIN_DENIED;
            refuse.error_code = 0; 
            session_write(session, &refuse, sizeof(refuse));

            return sizeof(PACKET_LOGIN_REQUEST);
        }

        std::cout << "[OasisLogin] >>> USUARIO AUTENTICADO COM SUCESSO! <<<" << std::endl;
        std::cout << "  -> Account ID vinculado: " << db_account_id << std::endl;

        // 1. Envia o pacote de confirmação de Login
        PACKET_LOGIN_ACCEPTED response{};
        response.packet_id = LOGIN_ACCEPTED;
        response.login_id1 = rand() % 999999;
        response.login_id2 = rand() % 999999;
        response.user_id = db_account_id;
        response.sex = (db_sex == "M") ? 1 : 0;

        std::cout << "[OasisLogin] Enviando LOGIN_ACCEPTED para FD: " << fd << std::endl;
        session_write(session, &response, sizeof(response));

        // 2. Envia o REDIRECIONAMENTO para o Char Server (0x0103)
        // Isso força o cliente a reconectar na porta correta (6121)
        PACKET_CHAR_SERVER_INFO char_info{};
        char_info.packet_id = CHAR_SERVER_INFO; // 0x0103
        char_info.ip = 0x0100007F;              // 127.0.0.1 (Little Endian)
        char_info.port = 6121;                  // Porta do seu Char Server
        
        std::cout << "[OasisLogin] Enviando instrucao de redirecionamento para porta 6121." << std::endl;
        session_write(session, &char_info, sizeof(char_info));

        return sizeof(PACKET_LOGIN_REQUEST);
    }

    default:
        std::cerr << "[OasisLogin] Pacote desconhecido (" << packet_id << ") de FD: " << fd << ". Derrubando cliente." << std::endl;
        return -1;
    }
}