#include <fsm_scanner.hpp>
#include <iostream>

// Protótipos de funções
int estadoInicial(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
int estadoNumero(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
int estadoOperador(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
int estadoPalavra(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
int estadoParenteses(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
int estadoVazio(const std::string &linha, size_t &pos, std::vector<std::string> &tokens);
bool isTokenIdentificador(const std::string &str);

int parseExpressao(std::string linha, std::vector<std::string> &tokens)
{
    size_t pos = 0;

    // Processa a linha enquanto tiver caracteres para ler
    while (pos < linha.length())
    {
        int status = estadoInicial(linha, pos, tokens);
        // Retorna se ocorrer um erro durante o processamento
        if (status != 0)
            return status;
    }
    return 0;
}

int estadoInicial(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    // Verifica se a posicao chegou no final da linha
    if (pos >= linha.length())
        return 0;

    char c = linha[pos];

    // Transições dos estados
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        return estadoVazio(linha, pos, tokens);
    }
    else if (c == '(' || c == ')')
    {
        return estadoParenteses(linha, pos, tokens);
    }
    else if (c >= '0' && c <= '9')
    {
        return estadoNumero(linha, pos, tokens);
    }
    else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^')
    {
        return estadoOperador(linha, pos, tokens);
    }
    else if ((c >= 'A' && c <= 'Z')) // Aceitar apenas letras maiusculas
    {
        return estadoPalavra(linha, pos, tokens);
    }

    std::cerr << "Token invalido '" << c << "' na posicao " << pos << "\n";
    return 1;
}

int estadoVazio(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    pos++;
    return 0;
}

int estadoNumero(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
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
                std::cerr << "Numero malformado em: " << buffer << c << "\n";
                return 1;
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
            std::cerr << "Lixo ou letra apos numero '" << c << "' na posicao " << pos << "\n";
            return 1;
        }
    }

    tokens.push_back(buffer);
    return 0;
}

int estadoOperador(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    std::string op = "";
    char c = linha[pos];
    op += c;
    pos++;

    // Tratar divisao inteira
    if (c == '/' && pos < linha.length() && linha[pos] == '/')
    {
        op += linha[pos];
        pos++;
    }
    tokens.push_back(op);
    return 0;
}

int estadoPalavra(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
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
            std::cerr << "Token invalido '" << c << "' na posicao " << pos << "\n";
            return 1;
        }
    }

    // Valida identificadores consecutivos
    if (!tokens.empty() && tokens.back() != "(" && tokens.back() != "RES")
    {
        // Verifica se o token anterior é um identificador
        if (isTokenIdentificador(tokens.back()))
        {
            // Se for um identificador, exibe o erro de identificadores consecutivos
            std::cerr << "Identificadores consecutivos " << tokens.back() << " e " << buffer << " na posicao " << pos - buffer.length() << "\n";
            return 1;
        }
    }

    // Adicionar o token no vetor se o buffer nao estiver vazio
    if (buffer.length() > 0)
    {
        tokens.push_back(buffer);
    }

    return 0;
}

int estadoParenteses(const std::string &linha, size_t &pos, std::vector<std::string> &tokens)
{
    std::string p = "";
    p += linha[pos];
    tokens.push_back(p);
    pos++;
    return 0;
}

bool isTokenIdentificador(const std::string &str)
{
    if (str.empty())
        return false;
    return (str[0] >= 'A' && str[0] <= 'Z');
}