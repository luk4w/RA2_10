#pragma once
#include <string>
#include <vector>

// Categorias para o gerador saber COMO navegar nos filhos
enum class ASTNodeType
{
    PROGRAMA,
    SEQUENCIA,
    INSTRUCAO_VFP,  // "VADD.F64" ou "VMUL.F64"
    MEMORIA_LOAD,   // "VLDR.F64", operando = "X"
    MEMORIA_STORE,  // "VSTR.F64", operando = "X"
    MEMORIA_RES,    // Historico de resultados (RES)
    NUMERO_LITERAL, // Folha da arvore, operando = "10.0"
    COMANDO_WHILE,
    COMANDO_IFELSE,
    INSTRUCAO_CMP
};

struct ASTNode
{
    ASTNodeType tipo;
    std::string opcode;   // "VADD.F64", "VDIV.F64"
    std::string operando; // "10.0", "VAR_X"

    std::vector<ASTNode *> filhos;

    // Construtor para facilitar a criacao de nos
    ASTNode(ASTNodeType t, std::string opc = "", std::string opnd = "") : tipo(t), opcode(opc), operando(opnd) {}

    // Destrutor para evitar memory leaks
    ~ASTNode()
    {
        for (ASTNode *filho : filhos)
        {
            delete filho;
        }
    }
};