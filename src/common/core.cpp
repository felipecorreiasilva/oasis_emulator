#include <iostream>
#include "socket.hpp"

// Função global que todos os servidores chamam ao ligar
void oasis_core_init() {
    std::cout << "==================================================" << std::endl;
    std::cout << "         Iniciando Oasis RPG Engine Core          " << std::endl;
    std::cout << "==================================================" << std::endl;

    if (!oasis_socket_init()) {
        std::cerr << "[Core] Erro fatal nos subsistemas de rede!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Função global chamada ao encerrar qualquer servidor
void oasis_core_shutdown() {
    oasis_socket_shutdown();
    std::cout << "[Core] Subsistemas encerrados com sucesso." << std::endl;
}