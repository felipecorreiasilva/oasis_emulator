#include "loginclif.hpp"
#include "../common/packets.hpp"
#include <iostream>

int parse_login(int fd, SessionData& session) {
    if (session.rdata.size() < 2) {
        return 0;
    }

    uint16_t packet_id = *reinterpret_cast<uint16_t*>(session.rdata.data());

    switch (packet_id) {
    case HEADER_PING: {
        std::cout << "[OasisLogin] Recebido comando PING do FD: " << fd << std::endl;
        uint16_t response = HEADER_PING;
        session_write(session, &response, sizeof(response));
        return 2;
    }

    case HEADER_CA_LOGIN: {
        if (session.rdata.size() < sizeof(p_ca_login)) {
            return 0;
        }

        p_ca_login* req = reinterpret_cast<p_ca_login*>(session.rdata.data());

        std::cout << "\n[OasisLogin] >>> REQUISICAO DE LOGIN DETECTADA! <<<" << std::endl;
        std::cout << "  -> Usuario:        " << req->username << std::endl;
        std::cout << "  -> Senha:          " << req->password << std::endl;
        std::cout << "  -> Versao Client:  " << req->client_version << std::endl;
        std::cout << "==================================================" << std::endl;

        // Simulação de resposta de sucesso (Padrão rAthena)
        p_ac_accept_login response{};
        response.packet_id = HEADER_AC_ACCEPT_LOGIN;
        response.login_id1 = 123456;
        response.login_id2 = 654321;
        response.user_id = 2000001;
        response.sex = 1;

        std::cout << "[OasisLogin] Enviando pacote de aceitacao (HEADER_AC_ACCEPT_LOGIN) para o FD: " << fd << std::endl;
        session_write(session, &response, sizeof(response));

        return sizeof(p_ca_login);
    }

    default:
        std::cerr << "[OasisLogin] Pacote desconhecido (" << packet_id << ") de FD: " << fd << ". Derrubando cliente." << std::endl;
        return -1;
    }
}