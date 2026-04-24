// Lucas Franco de Mello - Luk4w
// Nome do grupo no Canvas: RA2 10

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "cli_controller.hpp"
#include "fsm_scanner.hpp"
#include "armv7_generator.hpp"
#include "parser.hpp"
#include "tokens.hpp"

using namespace std;

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

    try
    {
        // Ler o arquivo tokens.txt salvo
        std::vector<TokenData> vtokens = lerTokens("tokens.txt");
        // for (const auto& t : vtokens) {
        //     std::cout << "Tipo: " << t.tipo << " | Valor: [" << t.valor << "]\n";
        // }
    }
    catch (std::exception &e)
    {
        cerr << "Falha ao ler arquivo de tokens" << endl;
        return 1;
    }

    string output;
    try
    {
        gerarAssembly(tokens_linha, output);
        // Gerar arquivo de saida .txt com o codigo assembly
        string nomeSaida = arq.substr(0, arq.length() - 4) + "_assembly.txt";
        ofstream outFile(nomeSaida);
        if (!outFile.is_open())
        {
            cerr << "nao foi possivel criar o arquivo " << nomeSaida << "\n";
            return 1;
        }
        outFile << output; // manda o buffer da ram pro disco rigido
        outFile.close();
        cout << "Codigo assembly gerado com sucesso em " << nomeSaida << "\n";
    }
    catch (std::exception &e)
    {
        cerr << "Erro ao gerar codigo assembly\n";
        return 1;
    }

    return 0;
}