#pragma once

#include "fsm_scanner.hpp"
#include "parser.hpp"
#include "gramatica.hpp"
#include "ast.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

// O 'inline' impede o erro de multipla definicao caso o main.cpp seja compilado mais de uma vez
inline void testeNumeros() {
    std::vector<std::string> tokens;
    parseExpressao("10.5 42", tokens, 1);
    
    assert(tokens.size() == 2);
    // Formato do scanner: "tipo,linha,valor" -> NUMERO(0), linha 1, valor 10.5
    assert(tokens[0] == "0,1,10.5");
    
    std::cout << "  -> [OK] Teste de Numeros\n";
}

inline void testeOperadores() {
    std::vector<std::string> tokens;
    parseExpressao("+ | /", tokens, 1);
    
    assert(tokens.size() == 3);
    assert(tokens[1] == "2,1,|");
    assert(tokens[2] == "2,1,/");
    
    std::cout << "  -> [OK] Teste de Operadores\n";
}

inline void testeOperadoresRelacionais(){
    std::vector<std::string> tokens;
    parseExpressao("> < <= >= != ==", tokens, 1);
    assert(tokens.size() == 6);

    std::cout << "  -> [OK] Teste de Operadores Relacionais\n";
}

// Funcao principal que voce vai chamar lá no main.cpp se vier com a flag test"
inline void executarTestesLexicos() {
    std::cout << "\nRODANDO TESTES UNITARIOS (LEXICO)\n";
    testeNumeros();
    testeOperadores();
    testeOperadoresRelacionais();
    std::cout << "\n[SUCESSO] Todos os testes lexicos passaram!\n\n";
}

// Helper q converte string em vector<TokenData> sem I/O de arquivo
// Formato do scanner "tipo,linha,valor"
inline std::vector<TokenData> _tokenizarEntrada(const std::string &entrada)
{
    std::vector<std::string> linhas;
    std::istringstream ss(entrada);
    std::string linha;
    while (std::getline(ss, linha))
        if (!linha.empty()) linhas.push_back(linha);

    std::vector<std::string> tokens_raw;
    for (int i = 0; i < (int)linhas.size(); ++i)
        parseExpressao(linhas[i], tokens_raw, i + 1);

    std::vector<TokenData> vtokens;
    for (const auto &t : tokens_raw)
    {
        size_t p1 = t.find(',');
        size_t p2 = t.find(',', p1 + 1);
        if (p1 != std::string::npos && p2 != std::string::npos)
        {
            TokenData td;
            td.tipo  = t.substr(0, p1);
            td.linha = std::stoi(t.substr(p1 + 1, p2 - p1 - 1));
            td.valor = t.substr(p2 + 1);
            vtokens.push_back(td);
        }
    }
    return vtokens;
}

// Helper tokeniza e parseia; lanca excecao em caso de erro sintatico/lexico
inline Derivacao _parsearEntrada(const std::string &entrada)
{
    auto vtokens = _tokenizarEntrada(entrada);
    return parsear(vtokens, tabela_ll1);
}

//- Casos validos-
inline void testeSintatico_SomaInteiros() {
    auto d = _parsearEntrada("(START)\n(3 2 +)\n(END)");
    assert(d.raiz != nullptr);
    assert(d.raiz->tipo == ASTNodeType::PROGRAMA);
    assert(d.raiz->filhos.size() == 1);
    assert(d.raiz->filhos[0]->tipo == ASTNodeType::INSTRUCAO_VFP);
    assert(d.raiz->filhos[0]->opcode == "VADD.F64");
    assert(d.raiz->filhos[0]->filhos[0]->operando == "3");
    assert(d.raiz->filhos[0]->filhos[1]->operando == "2");
    delete d.raiz;
    std::cout << "  -> [OK] Soma de inteiros: (3 2 +)\n";
}

inline void testeSintatico_Subtracao() {
    auto d = _parsearEntrada("(START)\n(10 4 -)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VSUB.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Subtracao: (10 4 -)\n";
}

inline void testeSintatico_Multiplicacao() {
    auto d = _parsearEntrada("(START)\n(5 6 *)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VMUL.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Multiplicacao: (5 6 *)\n";
}

inline void testeSintatico_DivisaoInteira() {
    auto d = _parsearEntrada("(START)\n(10 3 /)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "DIV_INT");
    delete d.raiz;
    std::cout << "  -> [OK] Divisao inteira: (10 3 /)\n";
}

inline void testeSintatico_DivisaoReal() {
    auto d = _parsearEntrada("(START)\n(10.0 3.0 |)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "VDIV.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Divisao real: (10.0 3.0 |)\n";
}

inline void testeSintatico_Modulo() {
    auto d = _parsearEntrada("(START)\n(10 3 %)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "MOD_INT");
    delete d.raiz;
    std::cout << "  -> [OK] Modulo: (10 3 %)\n";
}

inline void testeSintatico_Potencia() {
    auto d = _parsearEntrada("(START)\n(2 3 ^)\n(END)");
    assert(d.raiz->filhos[0]->opcode == "POW");
    delete d.raiz;
    std::cout << "  -> [OK] Potencia: (2 3 ^)\n";
}

inline void testeSintatico_NumeroReal() {
    auto d = _parsearEntrada("(START)\n(3.14 2.0 +)\n(END)");
    assert(d.raiz != nullptr);
    auto *op = d.raiz->filhos[0];
    assert(op->tipo == ASTNodeType::INSTRUCAO_VFP);
    assert(op->filhos[0]->tipo == ASTNodeType::NUMERO_LITERAL);
    assert(op->filhos[0]->operando == "3.14");
    assert(op->filhos[1]->operando == "2.0");
    delete d.raiz;
    std::cout << "  -> [OK] Numero real: (3.14 2.0 +)\n";
}

inline void testeSintatico_ExpressaoAninhada() {
    // ((10 5 *) 2 +)  =>  VADD( VMUL(10,5), 2 )
    auto d = _parsearEntrada("(START)\n((10 5 *) 2 +)\n(END)");
    assert(d.raiz != nullptr);
    auto *soma = d.raiz->filhos[0];
    assert(soma->opcode == "VADD.F64");
    assert(soma->filhos[0]->opcode == "VMUL.F64");
    assert(soma->filhos[0]->filhos[0]->operando == "10");
    assert(soma->filhos[0]->filhos[1]->operando == "5");
    delete d.raiz;
    std::cout << "  -> [OK] Expressao aninhada 2 niveis: ((10 5 *) 2 +)\n";
}

inline void testeSintatico_AninhamentoProfundo() {
    // (((2 2 ^) 2 ^) 2 ^)  =>  POW( POW( POW(2,2), 2 ), 2 )
    auto d = _parsearEntrada("(START)\n(((2 2 ^) 2 ^) 2 ^)\n(END)");
    assert(d.raiz != nullptr);
    auto *raiz_op = d.raiz->filhos[0];
    assert(raiz_op->opcode == "POW");
    assert(raiz_op->filhos[0]->opcode == "POW");
    assert(raiz_op->filhos[0]->filhos[0]->opcode == "POW");
    delete d.raiz;
    std::cout << "  -> [OK] Aninhamento profundo 3 niveis: (((2 2 ^) 2 ^) 2 ^)\n";
}

inline void testeSintatico_ComandoRES() {
    auto d = _parsearEntrada("(START)\n(10 5 +)\n(0 RES)\n(END)");
    assert(d.raiz != nullptr);
    assert(d.raiz->filhos.size() == 2);
    assert(d.raiz->filhos[1]->tipo == ASTNodeType::MEMORIA_RES);
    assert(d.raiz->filhos[1]->filhos[0]->operando == "0");
    delete d.raiz;
    std::cout << "  -> [OK] Comando RES: (0 RES)\n";
}

inline void testeSintatico_StoreLoad() {
    auto d = _parsearEntrada("(START)\n(42.0 VAR)\n(VAR)\n(END)");
    assert(d.raiz != nullptr);
    assert(d.raiz->filhos.size() == 2);
    assert(d.raiz->filhos[0]->tipo == ASTNodeType::MEMORIA_STORE);
    assert(d.raiz->filhos[0]->operando == "VAR");
    assert(d.raiz->filhos[1]->tipo == ASTNodeType::MEMORIA_LOAD);
    assert(d.raiz->filhos[1]->operando == "VAR");
    delete d.raiz;
    std::cout << "  -> [OK] Store/Load: (42.0 VAR) / (VAR)\n";
}

inline void testeSintatico_IFELSE() {
    // ((10 10 ==) (1 RES) (0 RES) IFELSE) => IFELSE com 3 filhos: cond / then / else
    auto d = _parsearEntrada("(START)\n((10 10 ==) (1 RES) (0 RES) IFELSE)\n(END)");
    assert(d.raiz != nullptr);
    auto *ifelse = d.raiz->filhos[0];
    assert(ifelse->tipo == ASTNodeType::COMANDO_IFELSE);
    assert(ifelse->filhos.size() == 3);
    assert(ifelse->filhos[0]->tipo == ASTNodeType::INSTRUCAO_CMP);  // condicao
    assert(ifelse->filhos[1]->tipo == ASTNodeType::MEMORIA_RES);    // then
    assert(ifelse->filhos[2]->tipo == ASTNodeType::MEMORIA_RES);    // else
    delete d.raiz;
    std::cout << "  -> [OK] IFELSE com 3 filhos (cond/then/else)\n";
}

inline void testeSintatico_WHILE() {
    auto d = _parsearEntrada("(START)\n(0 CONT)\n(((CONT) 3 <) (((CONT) 1 +) CONT) WHILE)\n(END)");
    assert(d.raiz != nullptr);
    assert(d.raiz->filhos.size() == 2);
    auto *whl = d.raiz->filhos[1];
    assert(whl->tipo == ASTNodeType::COMANDO_WHILE);
    assert(whl->filhos.size() == 2);
    assert(whl->filhos[0]->tipo == ASTNodeType::INSTRUCAO_CMP);  // condicao
    delete d.raiz;
    std::cout << "  -> [OK] WHILE com 2 filhos (cond/corpo)\n";
}

inline void testeSintatico_TodosOperadoresRelacionais() {
    const std::vector<std::string> ops = {"==", "!=", "<", ">", "<=", ">="};
    for (const auto &op : ops) {
        auto d = _parsearEntrada("(START)\n((1 2 " + op + ") (1 RES) (0 RES) IFELSE)\n(END)");
        assert(d.raiz != nullptr);
        assert(d.raiz->filhos[0]->filhos[0]->tipo == ASTNodeType::INSTRUCAO_CMP);
        assert(d.raiz->filhos[0]->filhos[0]->operando == op);
        delete d.raiz;
    }
    std::cout << "  -> [OK] Todos os operadores relacionais: ==  !=  <  >  <=  >=\n";
}

inline void testeSintatico_MultiplaExpressoes() {
    auto d = _parsearEntrada("(START)\n(1 2 +)\n(3 4 *)\n(5 6 -)\n(END)");
    assert(d.raiz != nullptr);
    assert(d.raiz->filhos.size() == 3);
    assert(d.raiz->filhos[0]->opcode == "VADD.F64");
    assert(d.raiz->filhos[1]->opcode == "VMUL.F64");
    assert(d.raiz->filhos[2]->opcode == "VSUB.F64");
    delete d.raiz;
    std::cout << "  -> [OK] Multiplas expressoes em sequencia\n";
}

inline void testeSintatico_ProgramaVazio() {
    auto d = _parsearEntrada("(START)\n(END)");
    assert(d.raiz != nullptr);
    assert(d.raiz->filhos.empty());
    delete d.raiz;
    std::cout << "  -> [OK] Programa vazio: (START)(END)\n";
}

inline void testeSintatico_Erro_OperadorNoInicio() {
    bool lancou = false;
    try { auto d = _parsearEntrada("(START)\n(+ 10 20)\n(END)"); delete d.raiz; }
    catch (const std::exception &) { lancou = true; }
    assert(lancou);
    std::cout << "  -> [OK] Erro: operador no inicio '(+ 10 20)'\n";
}

inline void testeSintatico_Erro_OperandosExcessivos() {
    bool lancou = false;
    try { auto d = _parsearEntrada("(START)\n(10 20 + 30)\n(END)"); delete d.raiz; }
    catch (const std::exception &) { lancou = true; }
    assert(lancou);
    std::cout << "  -> [OK] Erro: operandos excessivos '(10 20 + 30)'\n";
}

inline void testeSintatico_Erro_SemSTART() {
    bool lancou = false;
    try { auto d = _parsearEntrada("(10 2 +)\n(END)"); delete d.raiz; }
    catch (const std::exception &) { lancou = true; }
    assert(lancou);
    std::cout << "  -> [OK] Erro: programa sem (START)\n";
}

inline void testeSintatico_Erro_SemEND() {
    bool lancou = false;
    try { auto d = _parsearEntrada("(START)\n(10 2 +)"); delete d.raiz; }
    catch (const std::exception &) { lancou = true; }
    assert(lancou);
    std::cout << "  -> [OK] Erro: programa sem (END)\n";
}

inline void testeSintatico_Erro_DoisOperandosSemOperador() {
    bool lancou = false;
    try { auto d = _parsearEntrada("(START)\n(10 20)\n(END)"); delete d.raiz; }
    catch (const std::exception &) { lancou = true; }
    assert(lancou);
    std::cout << "  -> [OK] Erro: dois operandos sem operador '(10 20)'\n";
}

inline void testeSintatico_Erro_IFELSEIncompleto() {
    bool lancou = false;
    try { auto d = _parsearEntrada("(START)\n((1 1 ==) (1 RES) IFELSE)\n(END)"); delete d.raiz; }
    catch (const std::exception &) { lancou = true; }
    assert(lancou);
    std::cout << "  -> [OK] Erro: IFELSE incompleto (falta else)\n";
}

inline void executarTestesSintaticos() {
    std::cout << "\nRODANDO TESTES UNITARIOS (SINTATICO)\n";
    std::cout << "Casos validos\n";
    testeSintatico_SomaInteiros();
    testeSintatico_Subtracao();
    testeSintatico_Multiplicacao();
    testeSintatico_DivisaoInteira();
    testeSintatico_DivisaoReal();
    testeSintatico_Modulo();
    testeSintatico_Potencia();
    testeSintatico_NumeroReal();
    testeSintatico_ExpressaoAninhada();
    testeSintatico_AninhamentoProfundo();
    testeSintatico_ComandoRES();
    testeSintatico_StoreLoad();
    testeSintatico_IFELSE();
    testeSintatico_WHILE();
    testeSintatico_TodosOperadoresRelacionais();
    testeSintatico_MultiplaExpressoes();
    testeSintatico_ProgramaVazio();
    std::cout << "Casos de erro\n";
    testeSintatico_Erro_OperadorNoInicio();
    testeSintatico_Erro_OperandosExcessivos();
    testeSintatico_Erro_SemSTART();
    testeSintatico_Erro_SemEND();
    testeSintatico_Erro_DoisOperandosSemOperador();
    testeSintatico_Erro_IFELSEIncompleto();
    std::cout << "\n[SUCESSO] Todos os testes sintaticos passaram!\n\n";
}