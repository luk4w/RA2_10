#ifndef ARMV7_ASMGENERATOR_HPP
#define ARMV7_ASMGENERATOR_HPP

#include <string>
#include <vector>

/**
 * @brief Gera codigo assembly ARMv7 a partir de tokens em Notação Polonesa Reversa (RPN)
 * @param tokens Vetor contendo os tokens da expressão (números, variáveis, operadores)
 * @param codigoAssembly Referência para string onde o código assembly gerado será armazenado
 * @return int Retorna 0 em caso de sucesso
 * @note Todos os valores são manipulados como double (64-bit floating-point) R10 mantem o ponteiro de topo da pilha de historico e no final o probrama trava com "_halt: B _halt" para evitar comportamento indefinido
 */
int gerarAssembly(const std::vector<std::string>& _tokens_, std::string& codigoAssembly);

#endif