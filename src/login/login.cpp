#include "../common/socket.hpp"
#include "loginclif.hpp" // Inclui a nova interface do cliente
#include <iostream>

int main() {
    if (!oasis_socket_init()) {
        std::cerr << "[OasisLogin] Erro critico ao ligar subsistemas de rede do Windows." << std::endl;
        return 1;
    }

    std::cout << "==================================================" << std::endl;
    std::cout << "         Oasis RPG Online - Login Server          " << std::endl;
    std::cout << "==================================================" << std::endl;

    // Vincula a função que agora mora dentro do loginclif.cpp
    default_parse_func = parse_login;

    socket_t listen_fd = make_listen_bind("ANY", 6900);
    if (listen_fd == INVALID_SOCKET) {
        std::cerr << "[OasisLogin] Nao foi possivel alocar a porta 6900." << std::endl;
        oasis_socket_shutdown();
        return 1;
    }

    std::cout << "[OasisLogin] Motor assincrono online. Aguardando pacotes..." << std::endl;

    do_sockets_main_loop(listen_fd);

    oasis_socket_shutdown();
    return 0;
}