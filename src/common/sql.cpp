#include "sql.hpp"
#include <iostream>

Sql::Sql() : handle(nullptr), result(nullptr) {
    // Inicializa o conector interno do MySQL
    handle = mysql_init(nullptr);
}

Sql::~Sql() {
    free_result();
    close();
}

bool Sql::connect(const std::string& host, const std::string& user, const std::string& password, const std::string& database, uint16_t port) {
    if (!handle) {
        return false;
    }

    // Tenta estabelecer a conexão real
    MYSQL* res = mysql_real_connect(handle, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0);

    if (!res) {
        std::cerr << "[Sql Error] Falha ao conectar: " << get_error() << std::endl;
        return false;
    }

    // Força o banco a conversar com o emulador usando UTF-8 puro (evita erros com nomes de personagens)
    mysql_set_character_set(handle, "utf8mb4");
    return true;
}

bool Sql::query(const std::string& qstring) {
    free_result(); // Garante que a memória anterior está limpa

    if (mysql_query(handle, qstring.c_str()) != 0) {
        std::cerr << "[Sql Error] Erro na query: " << get_error() << std::endl;
        return false;
    }
    return true;
}

bool Sql::query_select(const std::string& qstring) {
    if (!query(qstring)) {
        return false;
    }

    // Armazena o resultado do SELECT na memória do emulador
    result = mysql_store_result(handle);
    if (!result && mysql_field_count(handle) > 0) {
        std::cerr << "[Sql Error] Falha ao armazenar resultado: " << get_error() << std::endl;
        return false;
    }
    return true;
}

MYSQL_ROW Sql::fetch_row() {
    if (!result) return nullptr;
    return mysql_fetch_row(result);
}

void Sql::free_result() {
    if (result) {
        mysql_free_result(result);
        result = nullptr;
    }
}

void Sql::close() {
    if (handle) {
        mysql_close(handle);
        handle = nullptr;
    }
}

std::string Sql::get_error() {
    if (handle) {
        return std::string(mysql_error(handle));
    }
    return "Instância do MySQL não inicializada.";
}