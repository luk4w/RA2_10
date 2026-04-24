// Lucas Franco de Mello - Luk4w
// Grupo: RA1 10

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "cli_controller.hpp"
#include "fsm_scanner.hpp"
#include "rpn_evaluator.hpp"
#include "armv7_generator.hpp"

using namespace std;

// Prototipos de funcao
void testarParseExpressao();
void exibirResultados();

int main(int argc, char *argv[])
{
    // Validacao do numero de argumentos
    if (argc != 2)
    {
        cerr << "Numero de argumentos invalidos\n";
        cerr << "Uso: " << argv[0] << " <teste.txt>\n";
        return 1;
    }

    string arq = argv[1];

    // Validacao de extensao .txt
    if (arq.length() < 4 || arq.substr(arq.length() - 4) != ".txt")
    {
        cerr << "A extensao do arquivo de entrada deve ser .txt\n";
        return 1;
    }

    // Alocar memoria dinamica para o buffer de linhas
    vector<string> buffer_linhas;

    // Extrair os dados do arquivo e armazenar no buffer
    int status = lerArquivo(arq, buffer_linhas);
    if (status != 0)
    {
        cerr << "Falha ao ler o arquivo " << arq << "\n";
        return status;
    }

    // Extrair os tokens de cada linha individualmente
    vector<string> tokens_linha;
    for (size_t i = 0; i < buffer_linhas.size(); ++i)
    {
        int status = parseExpressao(buffer_linhas[i], tokens_linha);
        if (status != 0)
        {
            cerr << "Erro lexico encontrado na linha " << i + 1 << endl;
        }
    }

    // testarParseExpressao();
    // exibirResultados();
    string output;
    if (gerarAssembly(tokens_linha, output))
    {
        cerr << "Erro ao gerar codigo assembly\n";
        return 1;
    }

    // Gerar arquivo de saida .txt com o codigo assembly
    string nomeSaida = arq.substr(0, arq.length() - 4) + "_output.txt";
    ofstream outFile(nomeSaida);

    if (!outFile.is_open())
    {
        cerr << "nao foi possivel criar o arquivo " << nomeSaida << "\n";
        return 1;
    }
    outFile << output; // manda o buffer da ram pro disco rigido
    outFile.close();

    cout << "Codigo assembly gerado com sucesso em " << nomeSaida << "\n";

    return 0;
}

// validar parseExpressao em casos validos e invalidos
void testarParseExpressao()
{
    struct CasoTeste
    {
        string entrada;
        int statusEsperado;
        size_t tokensEsperados;
        string descricao;
    };

    vector<CasoTeste> testes = {
        // Entradas validas
        {"(3.14 2.0 +)", 0, 5, "['(', '3.14', '2.0', '+', ')']"},
        {"(5 RES)", 0, 4, "['(', '5', 'RES', ')']"},
        {"(10.5 CONTADOR)", 0, 4, "['(', '10.5', 'CONTADOR', ')']"},
        {"((CONTADOR) 2.0 *)", 0, 7, "['(', '(', 'CONTADOR', ')', '2.0', '*', ')']"},
        {"((1 2 +) 3 *)", 0, 9, "['(', '(', '1', '2', '+', ')', '3', '*', ')']"},
        {"(10 3 //)", 0, 5, "['(', '10', '3', '//', ')']"},

        // Entradas invalidas
        {"(3.14 2.0 &)", 1, 0, "Erro lexico: Caractere nao reconhecido '&'"},
        {"3.14.5", 1, 0, "Erro lexico: Multiplos pontos flutuantes no literal"},
        {"3,45", 1, 0, "Erro lexico: Uso de virgula no lugar de ponto"},
        {"(10.5 BRASIL1)", 1, 0, "Erro lexico: Identificador alfanumerico invalido (numero no sufixo)"},
        {"(10.5 123ABC +)", 1, 0, "Erro lexico: Identificador alfanumerico invalido (numero no prefixo)"},
        {"(10.5 ABC# +)", 1, 0, "Erro lexico: Identificador com caractere especial '#'"},
        {"(10.5 ABC DEF +)", 1, 0, "Erro sintatico/lexico: Multiplos identificadores consecutivos"},
        {"(10.5 ABC+)", 1, 0, "Erro lexico: Ausencia de delimitador de espaco entre tokens"}};

    cout << "\n--- Validacao parseExpressao ---\n";
    for (const auto &t : testes)
    {
        vector<string> tokens;
        int status = parseExpressao(t.entrada, tokens);

        if (status == t.statusEsperado)
        {
            cout << "[OK] " << t.entrada << " --> " << t.descricao << endl
                 << endl;
        }
        else
        {
            cout << "[NOK] " << t.entrada << " --> " << t.descricao
                 << " Esperado: " << t.statusEsperado
                 << ", Obtido: " << status << "\n";
        }
    }
}

// validar executarExpressao no historico e memeoria
void exibirResultados()
{
    cout << "\n--- Validacao executarExpressao ---\n";

    std::vector<double> historico;
    std::map<std::string, double> memoria;

    // Teste 1: Conta Simples
    std::vector<std::string> tokens1;
    parseExpressao("(10.0 2.0 +)", tokens1);
    executarExpressao(tokens1, historico, memoria);
    std::cout << "Teste 1 (10 2 +): " << (historico.back() == 12.0 ? "[OK]" : "[NOK]") << " -> " << historico.back() << "\n";

    // Teste 2: Memoria (Store e Load)
    std::vector<std::string> tokens2;
    parseExpressao("(42.0 X)", tokens2);
    executarExpressao(tokens2, historico, memoria);
    std::cout << "Teste 2a (42 X): " << (memoria["X"] == 42.0 ? "[OK]" : "[NOK]") << " -> " << memoria["X"] << "\n";

    std::vector<std::string> tokens3;
    parseExpressao("((X) 2.0 /)", tokens3);
    executarExpressao(tokens3, historico, memoria);
    std::cout << "Teste 2b ((X) 2 /): " << (historico.back() == 21.0 ? "[OK]" : "[NOK]") << " -> " << historico.back() << "\n";

    // Teste 3: Historico (RES)
    std::vector<std::string> tokens4;
    parseExpressao("(1 RES)", tokens4);
    executarExpressao(tokens4, historico, memoria);
    std::cout << "Teste 3 (1 RES): " << (historico.back() == 12.0 ? "[OK]" : "[NOK]") << " -> " << historico.back() << "\n";

    // Teste 4: Erro Lexico - forçado token invalido
    std::vector<std::string> tokens5 = {"10.0", "X+", "+"};
    std::cout << "Teste 4 (Forcar identificador Invalido): ";
    if (executarExpressao(tokens5, historico, memoria) != 0)
    {
        std::cout << "[OK]\n";
    }
    else
    {
        std::cout << "[NOK]\n";
    }
}