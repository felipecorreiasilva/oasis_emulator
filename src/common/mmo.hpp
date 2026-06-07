#ifndef OASIS_MMO_HPP
#define OASIS_MMO_HPP

#include <stdint.h>

// Definições de tamanhos e limites do servidor (Idêntico ao rAthena)
constexpr int NAME_LENGTH = 24;
constexpr int MAX_CHARS_PER_ACCOUNT = 9;

// Sexo do Personagem
enum class enum_sex : uint8_t {
    sex_female = 0,
    sex_male = 1,
    sex_server = 2
};

// IDs das Classes Originais do Ragnarok (Mantido no Oasis)
enum class enum_class : uint16_t {
    class_novice = 0,  // O seu Aprendiz Inicial 2.5D
    class_swordman = 1,  // Espadachim
    class_mage = 2,  // Mago
    class_archer = 3,  // Arqueiro
    class_acolyte = 4,  // Noviço
    class_merchant = 5,  // Mercador
    class_thief = 6,  // Gatuno

    // Primeiras Classes Avançadas (Exemplos de ID padrão rAthena)
    class_knight = 7,  // Cavaleiro
    class_priest = 8,  // Sacerdote
    class_wizard = 9,  // Bruxo
    class_assassin = 12, // Mercenário
};

// Estrutura Mestre de Status do Personagem (O seu mmo_charstatus padrão rAthena)
struct mmo_charstatus {
    uint32_t char_id;
    uint32_t account_id;
    char name[NAME_LENGTH];

    enum_class class_;     // Mantém o padrão class_ com ID idêntico ao rag
    uint32_t base_level;
    uint32_t job_level;
    uint64_t base_exp;
    uint64_t job_exp;
    int32_t zeny;

    int32_t hp;
    int32_t max_hp;
    int32_t sp;
    int32_t max_sp;

    // Atributos Clássicos fundamentais do rAthena
    uint16_t str; // Força
    uint16_t agi; // Agilidade (Modifica a velocidade física float na Unity)
    uint16_t vit; // Vitalidade
    uint16_t int_;// Inteligência
    uint16_t dex; // Destreza
    uint16_t luk; // Sorte

    // INFRAESTRUTURA MODERNIZADA: Coordenadas salvas em Ponto Flutuante (float)
    float last_x;
    float last_y;
    uint16_t map_id;

    uint8_t slot; // Slot do personagem na tela de seleção da Unity (0 a 8)
};

#endif // OASIS_MMO_HPP