#include "parser.hpp"
#include <iostream>
#include <stdexcept>

using namespace std;

// Mapeamento dos símbolos da gramática para os nós da AST
ASTNodeType mapearTipoAST(const string &simbolo)
{
    if (simbolo == "programa")
        return ASTNodeType::PROGRAMA;
    if (simbolo == "NUMERO")
        return ASTNodeType::NUMERO_LITERAL;
    if (simbolo == "OPERADOR")
        return ASTNodeType::INSTRUCAO_VFP;
    if (simbolo == "OPERADOR_RELACIONAL")
        return ASTNodeType::INSTRUCAO_CMP;
    if (simbolo == "WHILE")
        return ASTNodeType::COMANDO_WHILE;
    if (simbolo == "IFELSE")
        return ASTNodeType::COMANDO_IFELSE;
    if (simbolo == "RES")
        return ASTNodeType::MEMORIA_RES;
    if (simbolo == "IDENTIFICADOR")
        return ASTNodeType::MEMORIA_LOAD;
    return ASTNodeType::SEQUENCIA;
}

// Tradutor da estrutura TokenData para terminais da gramática
string decodificarToken(const TokenData &token, string &valorReal)
{
    if (token.tipo == "$")
    {
        valorReal = "$";
        return "$";
    }

    int tipoInt = stoi(token.tipo);
    valorReal = token.valor;

    switch (tipoInt)
    {
    case 0:
        return "NUMERO";
    case 1:
        return "IDENTIFICADOR";
    case 2:
        return "OPERADOR";
    case 3:
        return valorReal; // START, END, WHILE, IFELSE, RES
    case 4:
        return "PARENTESE_ESQ";
    case 5:
        return "PARENTESE_DIR";
    case 6:
        return "OPERADOR_RELACIONAL";
    default:
        return "";
    }
}

struct ItemPilha
{
    string simbolo;
    ASTNode *nodo;
};

ASTNode *parsear(const vector<TokenData> &tokens,
                 const map<string, map<string, vector<string>>> &tabela_ll1)
{
    stack<ItemPilha> pilha;
    ASTNode *raiz = new ASTNode(ASTNodeType::PROGRAMA, "programa");

    pilha.push({"$", nullptr});
    pilha.push({"programa", raiz});

    size_t indexToken = 0;
    string valorReal;
    TokenData eofToken = {"$", "$"};

    string terminalAtual = (tokens.empty()) ? "$" : decodificarToken(tokens[indexToken], valorReal);

    cout << "Processando automato com pilha LL(1)...\n";

    while (!pilha.empty())
    {
        ItemPilha topo = pilha.top();
        pilha.pop();

        // Condição de parada (Match EOF)
        if (topo.simbolo == "$")
        {
            if (terminalAtual == "$")
            {
                // cout << "Cadeia sintatica aceita pelo automato.\n";
                break;
            }
            else
            {
                throw runtime_error("Erro de sintaxe: Esperado EOF, porem encontrado: " + valorReal);
            }
        }

        bool isTerminal = (tabela_ll1.find(topo.simbolo) == tabela_ll1.end());

        if (isTerminal)
        {
            // Match de terminais
            if (topo.simbolo == terminalAtual)
            {
                if (topo.nodo)
                {
                    topo.nodo->operando = valorReal;
                    // Definição de Opcodes ARMv7
                    if (topo.simbolo == "OPERADOR")
                    {
                        if (valorReal == "+")
                            topo.nodo->opcode = "VADD.F64";
                        else if (valorReal == "-")
                            topo.nodo->opcode = "VSUB.F64";
                        else if (valorReal == "*")
                            topo.nodo->opcode = "VMUL.F64";
                        else if (valorReal == "/")
                            topo.nodo->opcode = "DIV_INT";
                        else if (valorReal == "|")
                            topo.nodo->opcode = "VDIV.F64";
                        else
                            topo.nodo->opcode = valorReal;
                    }
                    if (topo.simbolo == "OPERADOR_RELACIONAL")
                        topo.nodo->opcode = valorReal;
                }

                indexToken++;
                if (indexToken < tokens.size())
                {
                    terminalAtual = decodificarToken(tokens[indexToken], valorReal);
                }
                else
                {
                    terminalAtual = decodificarToken(eofToken, valorReal);
                }
            }
            else
            {
                throw runtime_error("Erro de sintaxe: Esperado terminal '" + topo.simbolo + "', lido '" + terminalAtual + "'");
            }
        }
        else
        {
            // Expansão de Não-Terminais via Tabela de Análise
            auto itTabela = tabela_ll1.at(topo.simbolo);
            if (itTabela.find(terminalAtual) != itTabela.end())
            {
                vector<string> producao = itTabela.at(terminalAtual);

                // cout << "Derivacao: " << topo.simbolo << " -> ";
                // for (const string &s : producao)
                //     cout << s << " ";
                // cout << "\n";

                // Empilhamento reverso
                for (int i = producao.size() - 1; i >= 0; --i)
                {
                    string simb = producao[i];
                    ASTNode *filho = new ASTNode(mapearTipoAST(simb), simb);

                    if (topo.nodo)
                    {
                        topo.nodo->filhos.insert(topo.nodo->filhos.begin(), filho);
                    }
                    pilha.push({simb, filho});
                }
            }
            else
            {
                throw runtime_error("Erro de sintaxe: Transicao falhou em M[" + topo.simbolo + "][" + terminalAtual + "]. Token " + valorReal);
            }
        }
    }
    return raiz;
}