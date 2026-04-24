#include <fsm_scanner.hpp>
#include <iostream>
#include <stdexcept>

// Protótipos de funções
void estadoInicial(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
void estadoNumero(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
void estadoOperador(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
void estadoIdentificador(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
void estadoParentese(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
void estadoVazio(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);

void parseExpressao(std::string linha, std::vector<std::string> &tokens)
{
    size_t pos = 0;

    // Processa a linha enquanto tiver caracteres para ler
    while (pos < linha.length())
    {
        estadoInicial(linha, pos, tokens);
    }
}

void estadoInicial(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    // Verifica se a posicao chegou no final da linha
    if (pos >= linha.length())
        return;

    char c = linha[pos];

    // Transições dos estados
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        return estadoVazio(linha, pos, tokens);
    }
    else if (c == '(' || c == ')')
    {
        return estadoParentese(linha, pos, tokens);
    }
    else if (c >= '0' && c <= '9')
    {
        return estadoNumero(linha, pos, tokens);
    }
    else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '|' || c == '%' || c == '^' || c == '<' || c == '>' || c == '=' || c == '!')
    {
        return estadoOperador(linha, pos, tokens);
    }
    else if (c >= 'A' && c <= 'Z')
    {
        return estadoIdentificador(linha, pos, tokens);
    }

    throw std::runtime_error("Token invalido '" + std::string(1, c) + "' na posicao " + std::to_string(pos));
}

void estadoVazio(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    pos++;
}

void estadoNumero(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    std::string buffer = "";
    bool flag_ponto_decimal = false;

    while (pos < linha.length())
    {
        char c = linha[pos];

        // Verificar se o caractere é um dígito ou um ponto decimal
        if (c >= '0' && c <= '9')
        {
            buffer += c;
            pos++;
        }
        else if (c == '.')
        {
            if (flag_ponto_decimal)
            {
                throw std::runtime_error("Numero malformado em: " + buffer + c + "\n");
            }
            flag_ponto_decimal = true;
            buffer += c;
            pos++;
        }
        // Abrange mais casos de fim de token nos arquivos de teste, txt
        else if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ')')
        {
            break;
        }
        else
        {
            // letra com operador ou numero com letra -> 123ABC ou 123+
            throw std::runtime_error(std::string("Lixo ou letra apos numero '") + c + "' na posicao " + std::to_string(pos) + "\n");
        }
    }

    tokens.push_back(std::to_string(static_cast<int>(TipoToken::NUMERO)) + "," + buffer);
    return;
}

void estadoOperador(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    char c = linha[pos];

    // Tenta casar os Operadores Relacionais de 2 caracteres (==, !=, <=, >=)
    if (pos + 1 < linha.length())
    {
        std::string op2 = linha.substr(pos, 2);
        if (op2 == "==" || op2 == "!=" || op2 == "<=" || op2 == ">=")
        {
            tokens.push_back(std::to_string(static_cast<int>(TipoToken::OPERADOR_RELACIONAL)) + "," + op2);
            pos += 2;
            return;
        }
    }

    // Tenta casar os Operadores Relacionais de 1 caractere (<, >)
    if (c == '<' || c == '>')
    {
        tokens.push_back(std::to_string(static_cast<int>(TipoToken::OPERADOR_RELACIONAL)) + "," + std::string(1, c));
        pos++;
        return;
    }

    // Tenta casar os Operadores Matemáticos (+, -, *, /, |, %, ^)
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '|' || c == '%' || c == '^')
    {
        // Regra especial para o sinal de menos (número negativo)
        if (c == '-')
        {
            if (pos + 1 < linha.length() && isdigit(linha[pos + 1]))
            {
                // Verifica se o token anterior não é um operando (número, identificador ou ')')
                bool is_operando_anterior = false;
                if (!tokens.empty())
                {
                    std::string ultimo_token = tokens.back();
                    is_operando_anterior = (ultimo_token.rfind(std::to_string(static_cast<int>(TipoToken::PARENTESE_DIR)) + ",", 0) == 0 ||
                                            ultimo_token.rfind(std::to_string(static_cast<int>(TipoToken::IDENTIFICADOR)) + ",", 0) == 0 ||
                                            ultimo_token.rfind(std::to_string(static_cast<int>(TipoToken::NUMERO)) + ",", 0) == 0);
                }

                if (!is_operando_anterior)
                {
                    return estadoNumero(linha, pos, tokens);
                }
            }
        }

        // Evitar // ou || perdidos da Fase 1
        if (pos + 1 < linha.length())
        {
            if (c == '/' && linha[pos + 1] == '/')
            {
                throw std::runtime_error(std::string("O operador '//' nao existe. Use '/' para divisao inteira ou '|' para real. Posicao: ") + std::to_string(pos) + "\n");
            }
            if (c == '|' && linha[pos + 1] == '|')
            {
                throw std::runtime_error(std::string("O operador '||' e invalido. Use '|' para divisao real. Posicao: ") + std::to_string(pos) + "\n");
            }
        }

        tokens.push_back(std::to_string(static_cast<int>(TipoToken::OPERADOR)) + "," + std::string(1, c));
        pos++;
        return; // Sucesso
    }

    // escreveu somente = em vez de == ou ! ao inves de != --> erro léxico
    if (c == '=' || c == '!')
    {
        throw std::runtime_error(std::string("Operador relacional incompleto ou invalido '") + c + "' na posicao " + std::to_string(pos) + "\n");
    }

    throw std::runtime_error("Operador desconhecido");
}

void estadoIdentificador(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    std::string buffer = "";

    // Ler o caracter enquanto não terminar a linha
    while (pos < linha.length())
    {
        char c = linha[pos];
        // Enquanto o caractere for uma letra maiúscula, adiciona ao buffer
        if (c >= 'A' && c <= 'Z')
        {
            buffer += c;
            pos++;
        }
        // espaço, tabulação, nova linha ou parentese, considera o fim do token
        else if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ')')
        {
            break;
        }
        else
        {
            throw std::runtime_error(std::string("Token invalido '") + c + "' na posicao " + std::to_string(pos) + "\n");
        }
    }

    // Valida identificadores consecutivos
    if (!tokens.empty())
    {
        const std::string &ultimo_token = tokens.back();
        bool is_operando_anterior = (ultimo_token.rfind(std::to_string(static_cast<int>(TipoToken::IDENTIFICADOR)) + ",", 0) == 0 ||
                                     ultimo_token.rfind(std::to_string(static_cast<int>(TipoToken::NUMERO)) + ",", 0) == 0);

        if (is_operando_anterior)
        {
            // Unica excecao: atribuicao (NUMERO IDENTIFICADOR) ou acesso (NUMERO RES)
            if (tokens.size() < 2 || tokens[tokens.size() - 2] != std::to_string(static_cast<int>(TipoToken::PARENTESE_ESQ)) + ",(")
            {
                throw std::runtime_error("Erro de sintaxe: '" + buffer + "' inesperado apos '" + ultimo_token + "' na posicao " + std::to_string(pos - buffer.length()) + "\n");
            }
        }
    }

    if (buffer.length() > 0)
    {
        // Verificar se é palavra reservada ou identificador comum
        if (isPalavraReservada(buffer))
            tokens.push_back(std::to_string(static_cast<int>(TipoToken::PALAVRA_RESERVADA)) + "," + buffer);
        else
            tokens.push_back(std::to_string(static_cast<int>(TipoToken::IDENTIFICADOR)) + "," + buffer);
    }

    return;
}

void estadoParentese(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    std::string p = "";
    p += linha[pos];
    if (p == "(")
    {
        tokens.push_back(std::to_string(static_cast<int>(TipoToken::PARENTESE_ESQ)) + ",(");
    }
    else
    {
        tokens.push_back(std::to_string(static_cast<int>(TipoToken::PARENTESE_DIR)) + ",)");
    }
    pos++;
    return;
}
