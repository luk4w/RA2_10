#ifndef FSM_SCANNER_HPP
#define FSM_SCANNER_HPP

#include <vector>
#include <string>
#include "tokens.hpp"

/**
 * @brief Função responsável por extrair os tokens de uma linha escrita em notação polonesa reversa (RPN)
 * @param linha A linha de expressão a ser analisada
 * @param tokens Referência para os tokens extraídos
 */
void parseExpressao(std::string linha, std::vector<std::string> &tokens);

#endif