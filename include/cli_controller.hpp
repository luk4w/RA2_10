#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <vector>
#include <string>

/**
 * @brief Função responsável por ler um arquivo de texto e armazenar as linhas em um vetor de strings.
 * @param nomeArquivo O nome do arquivo a ser lido.
 * @param linhas Referencia para o vetor destino das strings extraidas do arquivo
 * @return Retorna 0 em caso de sucesso, ou 1 em caso de erro.
 */
int lerArquivo(std::string nomeArquivo, std::vector<std::string> &linhas);

#endif