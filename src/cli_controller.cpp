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
    if (!arquivo.is_open())
    {
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + nomeArquivo);
    }

    std::vector<TokenData> tokens_estruturados;
    std::string linhaLida;

    while (std::getline(arquivo, linhaLida))
    {
        if (linhaLida.empty())
            continue;

        // Encontra as posições das vírgulas para separar TIPO, LINHA e VALOR
        size_t pos1 = linhaLida.find(',');
        size_t pos2 = linhaLida.find(',', pos1 + 1);

        if (pos1 != std::string::npos && pos2 != std::string::npos)
        {
            TokenData td;
            td.tipo = linhaLida.substr(0, pos1);
            td.linha = std::stoi(linhaLida.substr(pos1 + 1, pos2 - pos1 - 1));
            td.valor = linhaLida.substr(pos2 + 1);

            tokens_estruturados.push_back(td);
        }
    }

    return tokens_estruturados;
}