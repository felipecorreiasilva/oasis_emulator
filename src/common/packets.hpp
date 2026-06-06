#ifndef OASIS_PACKETS_HPP
#define OASIS_PACKETS_HPP

#include <cstdint>

// ============================================================================
// OPCODES DE REDE (IDs dos Pacotes)
// Seguindo o padrão de nomenclatura Origem_Destino do rAthena
// ============================================================================
enum e_packet_id : uint16_t {
    // Custom/Testes
    HEADER_PING = 0x0001,

    // Oficiais/Adaptados (Exemplo de mapeamento do rAthena)
    HEADER_CA_LOGIN = 0x0064, // Cliente requisitando Login (Tradicional 0x0064)
    HEADER_AC_ACCEPT_LOGIN = 0x0069, // Login aceito, envia lista de Char-Servers
    HEADER_AC_REFUSE_LOGIN = 0x006a  // Login recusado (Senha errada, banido, etc)
};

#pragma pack(push, 1)

// ============================================================================
// ESTRUTURAS DE PACOTES (PACKETS)
// ============================================================================

// Pacote de Requisição de Login (Cliente -> Servidor)
struct p_ca_login {
    uint16_t packet_id;       // ID do pacote (HEADER_CA_LOGIN)
    char username[24];         // ID da conta do jogador
    char password[24];         // Senha em texto plano ou md5
    uint32_t client_version;   // Versão do Executável (Essencial no rAthena para rejeitar clients desatualizados)
    uint8_t clienttype;        // Tipo de cliente (Normal, Premium, etc)
};

// Pacote de Resposta de Sucesso (Servidor -> Cliente)
// O rAthena envia as informações de login e a lista de servidores disponíveis
struct p_ac_accept_login {
    uint16_t packet_id;       // HEADER_AC_ACCEPT_LOGIN
    uint32_t login_id1;        // ID único da sessão gerado pelo servidor
    uint32_t login_id2;        // ID secundário de autenticação
    uint32_t user_id;          // ID da conta no Banco de Dados (account_id)
    uint8_t sex;               // Sexo da conta (0: Feminino, 1: Masculino)

    // Na evolução do Oasis, adicionaremos um array de servidores aqui (Char IP/Porta)
};

// Pacote de Resposta de Falha (Servidor -> Cliente)
struct p_ac_refuse_login {
    uint16_t packet_id;       // HEADER_AC_REFUSE_LOGIN
    uint8_t error_code;        // Código do erro (0: Senha incorreta, 1: Conta Inexistente, 6: Banido)
};

#pragma pack(pop)

#endif // OASIS_PACKETS_HPP