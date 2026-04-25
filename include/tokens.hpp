#pragma once
#include <string>
#include <unordered_set>

enum class TipoToken
{
    NUMERO = 0,
    IDENTIFICADOR = 1,
    OPERADOR = 2,
    PALAVRA_RESERVADA = 3,
    PARENTESE_ESQ = 4,
    PARENTESE_DIR = 5,
    OPERADOR_RELACIONAL = 6
};

// Estrutura para facilitar a manipulação posterior
struct TokenData {
    std::string tipo;
    std::string valor;
    int linha;
};

inline const std::string T_NUMERO = std::to_string(static_cast<int>(TipoToken::NUMERO));
inline const std::string T_IDENTIFICADOR = std::to_string(static_cast<int>(TipoToken::IDENTIFICADOR));
inline const std::string T_OPERADOR = std::to_string(static_cast<int>(TipoToken::OPERADOR));
inline const std::string T_PALAVRA_RES = std::to_string(static_cast<int>(TipoToken::PALAVRA_RESERVADA));
inline const std::string T_PAREN_ESQ = std::to_string(static_cast<int>(TipoToken::PARENTESE_ESQ));
inline const std::string T_PAREN_DIR = std::to_string(static_cast<int>(TipoToken::PARENTESE_DIR));

inline const std::unordered_set<std::string> PALAVRAS_RESERVADAS = {
    "RES",
    "START",
    "END",
    "IFELSE",
    "WHILE",
};

inline bool isPalavraReservada(const std::string &s)
{
    return PALAVRAS_RESERVADAS.count(s) > 0;
}