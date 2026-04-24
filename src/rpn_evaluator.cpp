#include "rpn_evaluator.hpp"

#include <stack>
#include <iostream>
#include <cmath>
#include <string>

bool isIdentificadorValido(const std::string &token);

int executarExpressao(const std::vector<std::string> &tokens, std::vector<double> &historico_resultados, std::map<std::string, double> &memoria)
{
    // notação polonesa reversa -> calculo -> resultado
    // (10.0 2.0 +) -> calcula -> 12.0
    // (12.0 15.0 -) -> calcula -> -3.0
    // (5.0 3.0 *) -> calcula -> 15.0
    // (10.0 2.0 /) -> calcula -> 5.0
    // (10.0 3.0 //) -> calcula -> 3.0
    // (10.0 3.0 %) -> calcula -> 1.0
    // (2.0 3.0 ^) -> calcula -> 8.0

    // ((N RES) 4.0 +) -> calcula -> (valor do historico_resultados[tamanho_do_array - 1 - N])
    // Toda vez que executarExpressao, o resultado fica salvo em historico_resultados
    // historico_resultados[0] = 12.0
    // historico_resultados[1] = -3.0
    // ...

    // STORE
    // (V MEM) -> salva o valor do topo da pilha na variavel MEM
    // (100.0 BRASIL) -> salva 100.0 na variavel BRASIL -> memoria["BRASIL"] = 100.0

    // LOAD
    // Leitura da variavel -> (MEM)
    // Funciona quando a variável vem logo após o parêntese de abertura '('.
    // Como nao tem valor antes dela, a maquina entende que e uma busca.
    // ((BRASIL) 50.0 +) -> busca memoria["BRASIL"] (100.0), empilha -> (100.0 50.0 +) -> calcula -> 150.0

    // (42.0 X) salva 42 em X
    // ((X) 2.0 /) -> busca memoria["X"] (42.0), empilha -> (42.0 2.0 /) -> calcula -> 21.0

    std::stack<double> pilha;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const std::string &token = tokens[i];

        // parenteses sao apenas delimitadores de escopo lexico
        // ignora pois foram extraidos na fase de tokenizacao e nao impactam na logica da RPN
        if (token == "(" || token == ")")
            continue;

        // representacao da ULA do simulador
        if (token == "+" || token == "-" || token == "*" || token == "/" ||
            token == "//" || token == "%" || token == "^")
        {
            // Validacao de aridade
            if (pilha.size() < 2)
            {
                std::cerr << "Operandos insuficientes pra operacao " << token << "\n";
                return 1;
            }

            // Pop dos operandos da stack (B sai primeiro da pilha)
            double b = pilha.top();
            pilha.pop();
            double a = pilha.top();
            pilha.pop();

            if (token == "+")
                pilha.push(a + b);
            else if (token == "-")
                pilha.push(a - b);
            else if (token == "*")
                pilha.push(a * b);
            else if (token == "/")
                try
                {
                    pilha.push(a / b);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Erro na divisao: " << e.what() << "\n";
                    return 1;
                }
            else if (token == "//")
            {
                try
                {
                    pilha.push(static_cast<double>(static_cast<long long>(a) / static_cast<long long>(b)));
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Erro na divisao inteira: " << e.what() << "\n";
                    return 1;
                }
            }
            else if (token == "%")
            {
                try
                {
                    pilha.push(static_cast<double>(static_cast<long long>(a) % static_cast<long long>(b)));
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Erro na operacao de modulo: " << e.what() << "\n";
                    return 1;
                }
            }
            else if (token == "^")
            {
                pilha.push(std::pow(a, b));
            }
        }
        // COMANDO ESPECIAL PARA ACESSO AO HISTORICO DE RESULTADOS
        else if (token == "RES")
        {
            if (pilha.size() == 0)
            {
                std::cerr << "Falta o indice N antes do comando RES\n";
                return 1;
            }
            int n = static_cast<int>(pilha.top());
            pilha.pop();

            int index = historico_resultados.size() - 1 - n;
            if (index < 0 || index >= static_cast<int>(historico_resultados.size()))
            {
                std::cerr << "Indice RES " << n << " fora de alcance no historico\n";
                return 1;
            }
            pilha.push(historico_resultados[index]);
        }
        else
        {
            try
            {
                // Tenta converter a string para double (IEEE 754 de 64 bits)
                double valor_numerico = std::stod(token);
                pilha.push(valor_numerico);
            }
            catch (const std::invalid_argument &)
            {
                // a string tem letras, nao e numero -> e uma variavel
                // (BRASIL
                // se o token anterior for '(' 
                if (i > 0 && tokens[i - 1] == "(")
                {
                    // e o atual começar com letra maiscula
                    // LOAD
                    if (isIdentificadorValido(token))
                    {
                        // Se X existir, empilha o valor de X
                        // Se não existir, o mapa cria X valendo 0.0 e empilha o 0.0
                        pilha.push(memoria[token]);
                    }
                    else
                    {
                        //(BRASIL123) (brasil) ou qualquer outra coisa que nao tiver apenas letras maiuscula
                        std::cerr << "Erro lexico: Identificador invalido ou reservado no LOAD '" << token << "'\n";
                        return 1;
                    }
                }
                else // BRASIL)
                {
                    // STORE
                    // Se não tiver o '(' na frente e for tudo maiuscula
                    if (isIdentificadorValido(token))
                    {
                        if (pilha.size() > 0)
                        {
                            double val = pilha.top();
                            pilha.pop();
                            memoria[token] = val;
                        }
                        else
                        {
                            std::cerr << "Erro: Stack underflow ao gravar em " << token << "\n";
                            return 1;
                        }
                    }
                    else
                    {
                        std::cerr << "Erro lexico: Identificador invalido ou reservado no STORE '" << token << "'\n";
                        return 1;
                    }
                }
            }
        }
    } // Fim do for

    // Gravar o resultado final no historico
    if (pilha.size() > 0)
    {
        historico_resultados.push_back(pilha.top());
    }

    return 0;
}

// Verifica se a variavel e valida (todas letras maiusculas)
bool isIdentificadorValido(const std::string &token)
{
    // Valida se o token e reservado
    if (token.empty() || token == "RES")
        return false;

    // Varre a string caractere por caractere (Tabela ASCII)
    for (size_t k = 0; k < token.length(); k++)
    {
        // falha se encontrar algum que nao seja maisculo
        if (!(token[k] >= 'A' && token[k] <= 'Z'))
            return false;
    }
    return true;
}