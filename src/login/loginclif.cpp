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

        // Convertendo os arrays de char estáticos para std::string tratáveis
        std::string user(req->username);
        std::string pass(req->password);

        std::cout << "\n[OasisLogin] >>> REQUISICAO DE LOGIN DETECTADA! <<<" << std::endl;
        std::cout << "  -> Usuario Digitado: " << user << std::endl;
        std::cout << "  -> Versao Client:    " << req->client_version << std::endl;
        std::cout << "==================================================" << std::endl;

        // 1. Monta a query baseada na estrutura exata do seu phpMyAdmin
        std::string query_str = "SELECT account_id, userid, user_pass, sex FROM login WHERE userid = '" + user + "' LIMIT 1";

        // 2. Executa a query através do gerenciador de banco conectado externamente
        if (!db_handle.query_select(query_str)) {
            std::cerr << "[OasisLogin] Erro critico ao executar query no banco de dados." << std::endl;
            return -1; // Derruba o cliente por falha de infraestrutura
        }

        // 3. Busca o resultado da linha (Read do CRUD)
        MYSQL_ROW row = db_handle.fetch_row();

        // VALIDAÇÃO A: O usuário existe no banco de dados?
        if (!row) {
            std::cout << "[OasisLogin] Falha de Autenticacao: Usuario '" << user << "' nao existe." << std::endl;

            PACKET_LOGIN_DENIED refuse{};
            refuse.packet_id = LOGIN_DENIED;
            refuse.error_code = 1; // Código 1: Conta Inexistente (Padrão rAthena)
            session_write(session, &refuse, sizeof(refuse));

            return sizeof(PACKET_LOGIN_REQUEST); // Consome o pacote e encerra o fluxo do bloco
        }

        // Recupera as colunas retornadas de acordo com as posições do SELECT
        uint32_t db_account_id = std::stoul(row[0]); // account_id
        std::string db_pass = row[2];                // user_pass
        std::string db_sex = row[3];                 // sex

        // VALIDAÇÃO B: A senha digitada é igual à salva no banco?
        if (pass != db_pass) {
            std::cout << "[OasisLogin] Falha de Autenticacao: Senha incorreta para o usuario '" << user << "'." << std::endl;

            PACKET_LOGIN_DENIED refuse{};
            refuse.packet_id = LOGIN_DENIED;
            refuse.error_code = 0; // Código 0: Senha incorreta (Padrão rAthena)
            session_write(session, &refuse, sizeof(refuse));

            return sizeof(PACKET_LOGIN_REQUEST);
        }

        // 4. SUCESSO COMPLETO! Monta e entrega o pacote de aceitação preenchido dinamicamente
        std::cout << "[OasisLogin] >>> USUARIO AUTENTICADO COM SUCESSO! <<<" << std::endl;
        std::cout << "  -> Account ID vinculado: " << db_account_id << std::endl;

        PACKET_LOGIN_ACCEPTED response{};
        response.packet_id = LOGIN_ACCEPTED;

        // IDs randômicos de segurança de sessão para o handshake posterior com o Char-Server
        response.login_id1 = rand() % 999999;
        response.login_id2 = rand() % 999999;

        response.user_id = db_account_id; // Entrega o account_id real vindo do banco
        response.sex = (db_sex == "M") ? 1 : 0; // Converte o char "M" do banco para o id aceito pelo emulador

        std::cout << "[OasisLogin] Enviando LOGIN_ACCEPTED real para o FD: " << fd << std::endl;
        session_write(session, &response, sizeof(response));

        return sizeof(PACKET_LOGIN_REQUEST);
    }

    default:
        std::cerr << "[OasisLogin] Pacote desconhecido (" << packet_id << ") de FD: " << fd << ". Derrubando cliente." << std::endl;
        return -1;
    }
}