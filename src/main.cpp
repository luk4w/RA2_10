// Integrantes do grupo (ordem alfabetica):
// Lucas Franco de Mello - luk4w
// Nome do grupo no Canvas: RA2 10

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "cli_controller.hpp"
#include "fsm_scanner.hpp"
#include "armv7_generator.hpp"
#include "tokens.hpp"
#include "gramatica.hpp"
#include "parser.hpp"
#include "testes.hpp"
#include "ast_exporter.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    // Carregar a gramatica // definir regras de producao, calcular FIRST/FOLLOW e a tabela LL(1)
    construirGramatica();

    // imprimir a tabela LL1 no formato de matriz M[nao_terminal, terminal] = regra de prod
    cout << "--- TABELA DE PARSING LL(1) ---\n";
    for (const auto& [nao_terminal, transicoes] : tabela_ll1) {
        for (const auto& [terminal, regra] : transicoes) {
            cout << "M[" << nao_terminal << ", " << terminal << "] = { ";
            for (const string& s : regra) cout << s << " ";
            cout << "}\n";
        }
    }
    cout << "\n\n";

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
    try
    {
        lerArquivo(arq, buffer_linhas);
    }
    catch (std::exception &e)
    {
        cerr << "Erro ao ler arquivo " << arq << "\n";
        return 1;
    }

    // Extrair os tokens de cada linha individualmente 
    vector<string> tokens_linha;
    try
    {
        for (size_t i = 0; i < buffer_linhas.size(); ++i)
        {
            parseExpressao(buffer_linhas[i], tokens_linha);
        }
    }
    catch (const std::exception &e)
    {
        cerr << "Erro lexico: " << e.what() << "\n";
        return 1;
    }

    try
    {
        // Salvar os tokens em um arquivo da ultima execucao
        ofstream tokenFile("tokens.txt");
        for (const auto &token : tokens_linha)
        {
            tokenFile << token << "\n";
        }
        tokenFile.close();
    }
    catch (std::exception &e)
    {
        cerr << "Falha ao salvar tokens " << e.what() << "\n";
        return 1;
    }

    std::vector<TokenData> vtokens;
    try
    {
        // Ler o arquivo tokens.txt salvo
        vtokens = lerTokens("tokens.txt");
    }
    catch (std::exception &e)
    {
        cerr << "Falha ao ler arquivo de tokens" << endl;
        return 1;
    }

    // executarTestesLexicos();

    // Produz a estrutura de derivacao
    Derivacao derivacao;
    ASTNode *arvore = nullptr;
    try
    {
        derivacao = parsear(vtokens, tabela_ll1);

        // Transforma a derivacao em arvore sintatica estruturada
        arvore = gerarArvore(derivacao);

        if (arvore)
        {
            // Exporta a AST em JSON
            exportarAST(arvore, "ast_saida.json");
        }
    }
    catch (const std::exception &e)
    {
        cerr << e.what() << "\n";
        return 1;
    }

    // Geracao de codigo Assembly ARMv7 para Cpulator-ARMv7 DEC1-SOC(v16.1)
    // a partir da AST
    try
    {
        std::string codigoAssembly;
        gerarAssembly(arvore, codigoAssembly);

        ofstream asmFile("saida.s");
        if (asmFile.is_open())
        {
            asmFile << codigoAssembly;
            asmFile.close();
            cout << "Assembly gerado com sucesso em: saida.s\n";
        }
        else
        {
            cerr << "Erro ao criar arquivo de saida Assembly.\n";
        }
    }
    catch (const std::exception &e)
    {
        cerr << "Erro na geracao do Assembly: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

