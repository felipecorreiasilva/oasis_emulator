#include "../common/socket.hpp"
#include "mapclif.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>

std::map<std::string, std::string> read_config(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "[OasisMap] Aviso: Nao foi possivel abrir o arquivo: " << filename << std::endl;
        return config;
    }

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '/' || line[0] == '#') continue;

        size_t delimiter_pos = line.find(':');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            config[key] = value;
        }
    }
    return config;
}

int main() {
    if (!oasis_socket_init()) {
        std::cerr << "[OasisMap] Erro critico ao ligar subsistemas de rede do Windows." << std::endl;
        return 1;
    }

    std::cout << "==================================================" << std::endl;
    std::cout << "         Oasis RPG Online - Map Server            " << std::endl;
    std::cout << "==================================================" << std::endl;

    std::map<std::string, std::string> conf = read_config("conf/map_oasis.conf");
    int port = conf.count("map_port") ? std::stoi(conf["map_port"]) : 5121;

    std::cout << "[OasisMap] Porta configurada: " << port << std::endl;

    default_parse_func = parse_map;

    socket_t listen_fd = make_listen_bind("ANY", static_cast<uint16_t>(port));
    if (listen_fd == INVALID_SOCKET) {
        std::cerr << "[OasisMap] Nao foi possivel alocar a porta " << port << "." << std::endl;
        oasis_socket_shutdown();
        return 1;
    }

    std::cout << "[OasisMap] Map Server online. Aguardando conexoes..." << std::endl;
    do_sockets_main_loop(listen_fd);

    oasis_socket_shutdown();
    return 0;
}
