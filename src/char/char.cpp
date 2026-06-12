#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "../common/mmo.hpp" // Incluindo para acessar a struct ServerConfig
#include "charclif.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <map>

// Definição da variável global (Aloca memória conforme declarado no mmo.hpp)
ServerConfig g_config = { "127.0.0.1", 5121 }; 

// Instância global do banco de dados para o Char Server usar
Sql db_handle;

std::map<std::string, std::string> read_config(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "[OasisChar] Aviso: Nao foi possivel abrir o arquivo: " << filename << std::endl;
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
        std::cerr << "[OasisChar] Erro critico ao ligar subsistemas de rede do Windows." << std::endl;
        return 1;
    }

    std::cout << "==================================================" << std::endl;
    std::cout << "         Oasis RPG Online - Char Server          " << std::endl;
    std::cout << "==================================================" << std::endl;

    // 1. Carregar configuração do próprio Char Server
    std::map<std::string, std::string> conf = read_config("conf/char_oasis.conf");
    int port = conf.count("char_port") ? std::stoi(conf["char_port"]) : 6121;

    // 2. Carregar configuração do Map Server para atualizar g_config
    std::map<std::string, std::string> map_conf = read_config("conf/map_oasis.conf");
    if (map_conf.count("map_port")) {
        g_config.map_port = static_cast<uint16_t>(std::stoi(map_conf["map_port"]));
        std::cout << "[OasisChar] Porta do Map Server carregada: " << g_config.map_port << std::endl;
    }

    std::string db_ip = conf.count("char_db_ip") ? conf["char_db_ip"] : "127.0.0.1";
    std::string db_user = conf.count("char_db_id") ? conf["char_db_id"] : "root";
    std::string db_pass = conf.count("char_db_pw") ? conf["char_db_pw"] : "";
    std::string db_name = conf.count("char_db_db") ? conf["char_db_db"] : "oasis_db";
    int db_port = conf.count("char_db_port") ? std::stoi(conf["char_db_port"]) : 3306;

    std::cout << "[OasisChar] Porta configurada: " << port << std::endl;
    std::cout << "[OasisChar] Conectando ao banco em " << db_ip << ":" << db_port << "..." << std::endl;
    
    if (!db_handle.connect(db_ip, db_user, db_pass, db_name, db_port)) {
        std::cerr << "[OasisChar] [Erro Critico] Falha na conexao com o banco de dados!" << std::endl;
        oasis_socket_shutdown();
        return 1;
    }
    
    std::cout << "[OasisChar] Conectado ao banco '" << db_name << "' com sucesso!" << std::endl;

    default_parse_func = parse_char;

    socket_t listen_fd = make_listen_bind("ANY", static_cast<uint16_t>(port));
    if (listen_fd == INVALID_SOCKET) {
        std::cerr << "[OasisChar] Nao foi possivel alocar a porta " << port << "." << std::endl;
        oasis_socket_shutdown();
        return 1;
    }

    std::cout << "[OasisChar] Char Server online. Aguardando conexoes..." << std::endl;
    do_sockets_main_loop(listen_fd);

    db_handle.close();
    oasis_socket_shutdown();
    return 0;
}