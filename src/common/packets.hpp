#ifndef OASIS_PACKETS_HPP
#define OASIS_PACKETS_HPP

#include <cstdint>

constexpr int MAX_CHAR_SLOTS = 9;

// ============================================================================
// OPCODES DE REDE (IDs dos Pacotes)
// ============================================================================
enum e_packet_id : uint16_t {
    COMMON_PING            = 0x0001, // Pacote de verificação de conexão (Keep-Alive).

    // Login (0x01xx)
    LOGIN_REQUEST          = 0x0100, // Cliente envia usuário, senha e versão do client para autenticação.
    LOGIN_ACCEPTED         = 0x0101, // Servidor confirma login e envia dados da conta
    LOGIN_DENIED           = 0x0102, // Servidor recusa login (erro de senha, conta inexistente, etc)

    // Character (0x02xx)
    CHAR_LIST_REQUEST      = 0x0200, // Cliente solicita a lista de personagens da conta
    CHAR_LIST              = 0x0201, // Servidor envia a lista de personagens para o cliente
    CHAR_SELECT            = 0x0202, // Cliente escolhe qual personagem deseja logar
    CHAR_CREATE            = 0x0203, // Cliente envia dados para criar um novo personagem
    CHAR_CREATE_SUCCESS    = 0x0204, // Servidor confirma criação bem-sucedida do personagem
    CHAR_CREATE_FAILED     = 0x0205, // Servidor informa falha na criação (nome ocupado, erro)
    ZONE_SERVER_INFO       = 0x0206, // Servidor envia IP/Porta do Map Server após escolha do char

    // Map (0x03xx)
    MAP_ENTER_REQUEST      = 0x0300, // Cliente solicita entrada no mapa após receber IP do Zone
    MAP_ENTER_ACCEPTED     = 0x0301  // Servidor confirma que o cliente pode entrar no mapa
};

#pragma pack(push, 1)

// ============================================================================
// LOGIN
// ============================================================================

struct PACKET_LOGIN_REQUEST {
    uint16_t packet_id;
    char username[24];
    char password[24];
    uint32_t client_version;
};

struct PACKET_LOGIN_ACCEPTED {
    uint16_t packet_id;
    uint32_t login_id1;
    uint32_t login_id2;
    uint32_t user_id;
    uint8_t sex;
};

struct PACKET_LOGIN_DENIED {
    uint16_t packet_id;
    uint8_t error_code;
};

// ============================================================================
// CHARACTER
// ============================================================================

struct PACKET_CHAR_LIST_REQUEST {
    uint16_t packet_id;
    uint32_t user_id;
};

struct PACKET_CHAR_LIST {
    uint16_t packet_id;
    uint8_t count;
};

struct PACKET_CHAR_LIST_ENTRY {
    uint32_t char_id;
    uint8_t char_num;
    char name[24];
    uint8_t level;
    uint8_t sex;
    uint8_t hair;
    uint16_t map_id;
};

struct PACKET_CHAR_SELECT {
    uint16_t packet_id;
    uint8_t slot;
};

struct PACKET_CHAR_CREATE {
    uint16_t packet_id;
    char name[24];
    uint8_t slot;
    uint16_t hair_color;
    uint16_t hair_style;
    uint32_t job;
    uint8_t sex;
};

struct PACKET_CHAR_CREATE_SUCCESS {
    uint16_t packet_id;
    uint8_t result;
    uint32_t char_id;
    char name[24];
};

struct PACKET_CHAR_CREATE_FAILED {
    uint16_t packet_id;
    uint8_t error_code;
};

struct PACKET_ZONE_SERVER_INFO {
    uint16_t packet_id;
    uint32_t char_id;
    char mapname[16];
    uint32_t ip;
    uint16_t port;
};

// ============================================================================
// MAP
// ============================================================================

struct PACKET_MAP_ENTER_REQUEST {
    uint16_t packet_id;
    uint32_t char_id;
    uint16_t map_id;
    float x;
    float y;
};

struct PACKET_MAP_ENTER_ACCEPTED {
    uint16_t packet_id;
    uint8_t status;
    uint16_t map_id;
    float x;
    float y;
};

#pragma pack(pop)

#endif // OASIS_PACKETS_HPP