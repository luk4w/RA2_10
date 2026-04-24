#ifndef CLI_CONTROLLER_HPP
#define CLI_CONTROLLER_HPP

#include <vector>
#include <string>
#include "tokens.hpp"

/**
 * @brief Função responsável por ler um arquivo de texto e armazenar as linhas em um vetor de strings.
 * @param nomeArquivo O nome do arquivo a ser lido.
 * @param linhas Referencia para o vetor destino das strings extraidas do arquivo
 */
void lerArquivo(std::string nomeArquivo, std::vector<std::string> &linhas);

/**
* @brief Função responsavel por ler o arquivo de tokens da fase 1
* @param nomeArquivo O nome do arquivo a ser lido.
*/
std::vector<TokenData> lerTokens(std::string nomeArquivo);

#endif