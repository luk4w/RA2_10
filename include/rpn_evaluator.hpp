#ifndef RPN_EVALUATOR_HPP
#define RPN_EVALUATOR_HPP

#include <vector>
#include <string>
#include <map>

/**
 * @brief Função responsavel por validar a notação polonesa reversa/codigo que vai gerar o assembly (simula o que a ULA do simulador ARMv7 vai processar)
 * @param tokens Os tokens da expressao a ser avaliada
 * @param historico_resultados armazenar os resultados das expressoes anteriores (N RES)
 * @param memoria hashmap para permitir o acesso as variaveis armazenadas (V MEM) e (MEM)
 * @return Retorna 0 em caso de sucesso, ou 1 em caso de erro
 */
int executarExpressao(const std::vector<std::string> &tokens, std::vector<double> &historico_resultados, std::map<std::string, double> &memoria);

#endif