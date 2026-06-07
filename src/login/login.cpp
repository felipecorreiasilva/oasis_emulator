#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "loginclif.hpp" // Interface do cliente para tratar pacotes de login
#include <iostream>
#include <fstream>
#include <string>
#include <map>

// Instância global do banco de dados para o Login Server usar
Sql db_handle;

// Função assíncrona para ler o arquivo .conf do Oasis
std::map<std::string, std::string> read_config(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "[OasisLogin] Aviso: Nao foi possivel abrir o arquivo: " << filename << std::endl;
        return config;
    }

    while (std::getline(file, line)) {
        // Ignora comentários e linhas vazias
        if (line.empty() || line[0] == '/' || line[0] == '#') continue;

        size_t delimiter_pos = line.find(':');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);

            // Limpa espaços extras ao redor das chaves e valores
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            config[key] = value;
        }
    }
    return config;
}

int main() {
    // 1. Inicializa os subsistemas de rede (WSAStartup no Windows)
    if (!oasis_socket_init()) {
        std::cerr << "[OasisLogin] Erro critico ao ligar subsistemas de rede do Windows." << std::endl;
        return 1;
    }

    std::cout << "==================================================" << std::endl;
    std::cout << "         Oasis RPG Online - Login Server          " << std::endl;
    std::cout << "==================================================" << std::endl;

    // 2. Carrega as configurações do banco de dados do arquivo login_oasis.conf
    std::map<std::string, std::string> conf = read_config("conf/login_oasis.conf");

    // Define os valores padrões caso o arquivo falte ou esteja incompleto
    std::string db_ip = conf.count("login_db_ip") ? conf["login_db_ip"] : "127.0.0.1";
    std::string db_user = conf.count("login_db_id") ? conf["login_db_id"] : "root";
    std::string db_pass = conf.count("login_db_pw") ? conf["login_db_pw"] : "";
    std::string db_name = conf.count("login_db_db") ? conf["login_db_db"] : "oasis_db";
    int db_port = conf.count("login_db_port") ? std::stoi(conf["login_db_port"]) : 3306;

    // 3. Conecta ao MySQL do XAMPP
    std::cout << "[OasisLogin] Conectando ao banco em " << db_ip << ":" << db_port << "..." << std::endl;
    if (!db_handle.connect(db_ip, db_user, db_pass, db_name, db_port)) {
        std::cerr << "[OasisLogin] [Erro Critico] Falha na conexao com o banco de dados!" << std::endl;
        oasis_socket_shutdown();
        return 1;
    }
    std::cout << "[OasisLogin] Conectado ao banco '" << db_name << "' com sucesso!" << std::endl;

    // 4. Vincula a função de processamento de pacotes que mora no loginclif.cpp
    default_parse_func = parse_login;

    // 5. Abre a porta 6900 para escuta de pacotes (Unity, Char-Server, etc.)
    socket_t listen_fd = make_listen_bind("ANY", 6900);
    if (listen_fd == INVALID_SOCKET) {
        std::cerr << "[OasisLogin] Nao foi possivel alocar a porta 6900." << std::endl;
        db_handle.close();
        oasis_socket_shutdown();
        return 1;
    }

    std::cout << "[OasisLogin] Motor assincrono online. Aguardando pacotes..." << std::endl;

    // 6. Entra no loop infinito de processamento de conexões de rede
    do_sockets_main_loop(listen_fd);

    // Finalização dos serviços se o loop for quebrado de forma segura
    db_handle.close();
    oasis_socket_shutdown();
    return 0;
}