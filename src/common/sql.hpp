#ifndef OASIS_SQL_HPP
#define OASIS_SQL_HPP

#include <string>

#include <mysql.h> // Cabeçalho nativo do MySQL

class Sql {
private:
    MYSQL* handle;      // Conexão interna com o banco (equivalente ao Sql->handle do rAthena)
    MYSQL_RES* result;  // Guarda o resultado da última query executada

public:
    Sql();
    ~Sql();

    // Conecta ao banco de dados utilizando as credenciais
    bool connect(const std::string& host, const std::string& user, const std::string& password, const std::string& database, uint16_t port);

    // Executa uma query que não retorna dados (INSERT, UPDATE, DELETE)
    bool query(const std::string& qstring);

    // Executa uma query que retorna dados (SELECT)
    bool query_select(const std::string& qstring);

    // Puxa a próxima linha do resultado (Equivalente ao SQL_SUCCESS do rAthena)
    MYSQL_ROW fetch_row();

    // Libera a memória do último resultado gerado
    void free_result();

    // Fecha a conexão ativa
    void close();

    // Retorna a mensagem do último erro ocorrido no banco
    std::string get_error();
};

#endif // OASIS_SQL_HPP