#include <cli_controller.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

void lerArquivo(std::string nomeArquivo, std::vector<std::string> &linhas)
{
    // input file stream
    std::ifstream arquivo(nomeArquivo);

    // Verificar se o arquivo foi aberto com sucesso
    if (!arquivo.is_open())
    {
        throw std::runtime_error("Nao foi possivel abrir o arquivo " + nomeArquivo);
    }
    std::string buffer_linha;

    // Extrair os caracteres do arquivo e armazenar no buffer de linhas
    while (std::getline(arquivo, buffer_linha))
    {
        // adicionar a linha lida ao vetor de linhas, caso a linha nao esteja vazia
        if (!buffer_linha.empty())
        {
            linhas.push_back(buffer_linha);
        }
    }
    // Libera o lock de leitura do arquivo
    arquivo.close();

    // Verificar se existem linhas depois de ler o arquivo
    if (linhas.empty())
    {
        throw std::runtime_error("O arquivo carregado esta vazio\n");
    }
    else if (linhas.size() < 10)
    {
        throw std::runtime_error("O arquivo carregado possui menos de 10 linhas. Linhas carregadas: " + std::to_string(linhas.size()));
    }
    else
    {
        std::cout << "Sucesso " << linhas.size() << " linhas carregadas.\n";
    }
};

std::vector<TokenData> lerTokens(std::string nomeArquivo)
{
    std::ifstream arquivo(nomeArquivo);
    // Verificar se o arquivo foi aberto com sucesso
    if (!arquivo.is_open())
    {
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + nomeArquivo);
    }


    std::vector<TokenData> tokens_estruturados;
    std::string linha;

    // Extrair o tipo e o valor do token e transformar num vetor estruturado
    while (std::getline(arquivo, linha))
    {
        if (linha.empty()) continue;

        // splitToken retorna std::pair<std::string, std::string> 
        auto [tipo, valor] = splitToken(linha); 
        
        tokens_estruturados.push_back({tipo, valor});
    }

    return tokens_estruturados;
};