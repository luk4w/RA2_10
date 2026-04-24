#ifndef ARMV7_ASMGENERATOR_HPP
#define ARMV7_ASMGENERATOR_HPP

#include <string>
#include <vector>
#include "ast.hpp"

/**
 * @brief Gera codigo assembly ARMv7 a partir da arvore sintatica (AST)
 * @param arvore Raiz da AST produzida por gerarArvore()
 * @param codigoAssembly Referencia para string onde o codigo assembly sera armazenado
 * @note Todos os valores sao manipulados como double (64-bit floating-point) R10 mantem o ponteiro de topo da pilha de historico (RES).
 */
void gerarAssembly(ASTNode *arvore, std::string &codigoAssembly);

#endif