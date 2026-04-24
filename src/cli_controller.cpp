#include <cli_controller.hpp>
#include <iostream>
#include <fstream>

int lerArquivo(std::string nomeArquivo, std::vector<std::string> &linhas)
{
    // input file stream
    std::ifstream arquivo(nomeArquivo);

    // Verificar se o arquivo foi aberto com sucesso
    if (!arquivo.is_open())
    {
        std::cerr << "Nao foi possivel abrir o arquivo " << nomeArquivo << "\n";
        return 1;
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
        std::cerr << "O arquivo carregado esta vazio\n";
        return 1;
    }
    else if (linhas.size() < 10)
    {
        std::cerr << "O arquivo carregado possui menos de 10 linhas. Linhas carregadas: " << linhas.size() << "\n";
        return 1;
    }
    else
    {
        std::cout << "Sucesso " << linhas.size() << " linhas carregadas.\n";
        return 0;
    }
};