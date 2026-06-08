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
    HEADER_CA_CHARLIST = 0x0065, // Cliente requisita lista de personagens (custom)
    HEADER_CH_SELECT_CHAR = 0x0066, // Cliente seleciona personagem (rAthena compatível)
    HEADER_CA_ENTER_MAP = 0x006b, // Cliente solicita entrada no mapa
    HEADER_AC_ENTER_MAP = 0x006c, // Servidor confirma entrada no mapa
    HEADER_AC_CHARLIST = 0x00A0, // Servidor responde com lista de personagens (custom)
    HEADER_AC_ACCEPT_LOGIN = 0x0069, // Login aceito, envia lista de Char-Servers
    HEADER_AC_REFUSE_LOGIN = 0x006a, // Login recusado (Senha errada, banido, etc)
    HEADER_HC_NOTIFY_ZONESVR = 0x0071 // Servidor informa IP/porta do Map Server
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

struct p_ca_charlist {
    uint16_t packet_id;       // HEADER_CA_CHARLIST
    uint32_t user_id;         // ID da conta autenticada
};

struct p_ac_charlist_header {
    uint16_t packet_id;       // HEADER_AC_CHARLIST
    uint8_t count;            // Número de personagens enviados
};

struct p_ac_charlist_entry {
    uint32_t char_id;         // ID interno do personagem
    char name[24];            // Nome do personagem
    uint8_t level;            // Nível do personagem
    uint16_t map_id;          // Mapa inicial
    uint16_t x;               // Posição X inicial
    uint16_t y;               // Posição Y inicial
};

struct p_ch_select_char {
    uint16_t packet_id;       // HEADER_CH_SELECT_CHAR
    uint8_t slot;             // Slot de personagem selecionado
};

struct p_hc_notify_zonesvr {
    uint16_t packet_id;       // HEADER_HC_NOTIFY_ZONESVR
    uint32_t char_id;         // ID interno do personagem
    char mapname[16];         // Nome do mapa destino
    uint32_t ip;              // IP do Map Server
    uint16_t port;            // Porta do Map Server
};

struct p_ca_enter_map {
    uint16_t packet_id;       // HEADER_CA_ENTER_MAP
    uint32_t char_id;         // Personagem entrando no mapa
    uint16_t map_id;          // Mapa solicitado
    uint16_t x;               // Posição X desejada
    uint16_t y;               // Posição Y desejada
};

struct p_ac_enter_map {
    uint16_t packet_id;       // HEADER_AC_ENTER_MAP
    uint8_t status;           // 1 = aceito, 0 = recusado
    uint16_t map_id;          // Mapa aceito
    uint16_t x;               // Posição X de entrada
    uint16_t y;               // Posição Y de entrada
};

#pragma pack(pop)

#endif // OASIS_PACKETS_HPP