#pragma once

#include "fsm_scanner.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

// O 'inline' impede o erro de multipla definicao caso o main.cpp seja compilado mais de uma vez
inline void testeNumeros() {
    std::vector<std::string> tokens;
    parseExpressao("10.5 42", tokens);
    
    assert(tokens.size() == 2); 
    // OBS: Ajuste a string abaixo para bater exatamente com o formato que o seu Lexico cospe
    assert(tokens[0] == "0,10.5"); 
    
    std::cout << "  -> [OK] Teste de Numeros\n";
}

inline void testeOperadores() {
    std::vector<std::string> tokens;
    parseExpressao("+ | /", tokens);
    
    assert(tokens.size() == 3);
    assert(tokens[1] == "2,|");
    assert(tokens[2] == "2,/");
    
    std::cout << "  -> [OK] Teste de Operadores\n";
}

inline void testeOperadoresRelacionais(){
    std::vector<std::string> tokens;
    parseExpressao("> < <= >= != ==", tokens);
    assert(tokens.size() == 6);

    std::cout << "  -> [OK] Teste de Operadores Relacionais\n";
}

// Funcao principal que voce vai chamar lá no main.cpp se vier com a flag "--test"
inline void executarTestesLexicos() {
    std::cout << "\nRODANDO TESTES UNITARIOS (LEXICO)\n";
    testeNumeros();
    testeOperadores();
    testeOperadoresRelacionais();
    std::cout << "\n[SUCESSO] Todos os testes lexicos passaram!\n\n";
}