#ifndef FSM_SCANNER_HPP
#define FSM_SCANNER_HPP

#include <vector>
#include <string>

/**
 * @brief Função responsável por extrair os tokens de uma linha escrita em notação polonesa reversa (RPN)
 * @param linha A linha de expressão a ser analisada
 * @param tokens Referência para os tokens extraídos
 * @return Retorna 0 em caso de sucesso, ou 1 em caso de erro
 */
int parseExpressao(std::string linha, std::vector<std::string>& tokens);

#endif