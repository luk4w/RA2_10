#include "armv7_generator.hpp"
#include <sstream>
#include <map>
#include <string>

// Como a AST foi projetada com opcodes ARMv7 VFP embutidos nos nós (VADD.F64, VLDR.F64)
// facilita a vida, ou seja
// primeiro emite o codigo dos filhos (deixando operandos na pilha FPU)
// em seguida o nó corrente emite sua propria instrucao

// A pre analise percorre a AST coletando literais numericos e variaveis
// isso sera usado para gerar a secao .data do Assembly
static void coletarLiteraisVariaveis(
    ASTNode *node,
    std::map<std::string, std::string> &literais,
    std::map<std::string, std::string> &variaveis,
    int &contagemLiterais)
{
    if (!node)
        return;

    switch (node->tipo)
    {
    case ASTNodeType::NUMERO_LITERAL:
        if (literais.find(node->operando) == literais.end())
            literais[node->operando] = "LIT_" + std::to_string(contagemLiterais++);
        break;
    case ASTNodeType::MEMORIA_LOAD:
    case ASTNodeType::MEMORIA_STORE:
        if (variaveis.find(node->operando) == variaveis.end())
            variaveis[node->operando] = "VAR_" + node->operando;
        break;
    default:
        break;
    }

    for (ASTNode *filho : node->filhos)
        coletarLiteraisVariaveis(filho, literais, variaveis, contagemLiterais);
}

// Decide se um no deixa um valor na pilha FPU apos sua emissao
// Usado para determinar se a expressao de topo deve ser gravada no historico
// de resultados (RES) ao final.
static bool produzValor(ASTNodeType tipo)
{
    switch (tipo)
    {
    case ASTNodeType::NUMERO_LITERAL:
    case ASTNodeType::MEMORIA_LOAD:
    case ASTNodeType::MEMORIA_RES:
    case ASTNodeType::INSTRUCAO_VFP:
        return true;
    default:
        return false;
    }
}

// Walker recursivo pos-ordem: emite o Assembly para o subarvore raiz em no
// Invariante: ao retornar, nos que "produzem valor" deixam 1 double na pilha FPU (via VPUSH.F64)
// Nos de efeito colateral (STORE) consomem e nao deixam residuo
// Comandos de controle (IFELSE/WHILE) serao completados na logica de controle e saltos do programa
static void emitirNo(
    ASTNode *node,
    std::stringstream &ss,
    const std::map<std::string, std::string> &literais,
    const std::map<std::string, std::string> &variaveis,
    int &contadorLabel)
{
    if (!node)
        return;

    switch (node->tipo)
    {
    case ASTNodeType::PROGRAMA:
        // Cada filho e uma expressao/comando de topo.
        for (ASTNode *filho : node->filhos)
        {
            emitirNo(filho, ss, literais, variaveis, contadorLabel);

            // Apos expressoes de topo que produzem valor, salva em RES.
            if (produzValor(filho->tipo))
            {
                ss << "    @ salva o resultado no historico de memoria\n";
                ss << "    VPOP.F64 {D0}           @ POP resultado final da FPU\n";
                ss << "    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico\n";
                ss << "    VSTR.F64 D0, [R10]      @ Grava na RAM\n\n";
            }
        }
        break;

    case ASTNodeType::NUMERO_LITERAL:
        // Folha: empilha a constante vinda da secao .data.
        ss << "    @ Empilha Constante: " << node->operando << "\n";
        ss << "    LDR R0, =" << literais.at(node->operando) << "\n";
        ss << "    VLDR.F64 D0, [R0]\n";
        ss << "    VPUSH.F64 {D0}\n\n";
        break;

    case ASTNodeType::MEMORIA_LOAD:
        // Folha: carrega o valor da variavel e empilha.
        // node->opcode = "VLDR.F64" (definido pelo parser).
        ss << "    @ LOAD: Le a variavel " << node->operando << " da memoria e empilha\n";
        ss << "    LDR R0, =" << variaveis.at(node->operando) << "\n";
        ss << "    " << node->opcode << " D0, [R0]\n";
        ss << "    VPUSH.F64 {D0}\n\n";
        break;

    case ASTNodeType::MEMORIA_STORE:
        // Emite o valor a armazenar (filho unico), depois grava na variavel.
        // node->opcode = "VSTR.F64" (definido pelo parser).
        if (!node->filhos.empty())
            emitirNo(node->filhos[0], ss, literais, variaveis, contadorLabel);
        ss << "    @ Grava resultado na variavel " << node->operando << "\n";
        ss << "    VPOP.F64 {D0}\n";
        ss << "    LDR R0, =" << variaveis.at(node->operando) << "\n";
        ss << "    " << node->opcode << " D0, [R0]\n\n";
        break;

    case ASTNodeType::MEMORIA_RES:
        // Emite o indice N (filho), depois resolve offset no historico.
        if (!node->filhos.empty())
            emitirNo(node->filhos[0], ss, literais, variaveis, contadorLabel);
        ss << "    @ Operador RES: Busca historico na memoria\n";
        ss << "    VPOP.F64 {D0}           @ Pega o N da pilha\n";
        ss << "    VCVT.S32.F64 S0, D0     @ Converte N de Double para Inteiro\n";
        ss << "    VMOV R1, S0             @ MOV N pro R1\n";
        ss << "    LSL R1, R1, #3          @ Multiplica N por 8 bytes (descobrir o offset)\n";
        ss << "    ADD R2, R10, R1         @ Soma o offset com o R10 para achar o endereco antigo\n";
        ss << "    VLDR.F64 D1, [R2]       @ Le o historico da RAM\n";
        ss << "    VPUSH.F64 {D1}          @ PUSH pilha\n\n";
        break;

    case ASTNodeType::INSTRUCAO_VFP:
        // Pos-ordem: emite os dois operandos primeiro, depois o operador.
        if (node->filhos.size() >= 2)
        {
            emitirNo(node->filhos[0], ss, literais, variaveis, contadorLabel);
            emitirNo(node->filhos[1], ss, literais, variaveis, contadorLabel);
        }

        // Operadores de instrucao unica: usa diretamente node->opcode.
        if (node->opcode == "VADD.F64")
        {
            ss << "    @ Soma\n";
            ss << "    VPOP.F64 {D1}           @ POP operando B\n";
            ss << "    VPOP.F64 {D0}           @ POP operando A\n";
            ss << "    VADD.F64 D2, D0, D1     @ A + B\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH pilha\n\n";
        }
        else if (node->opcode == "VSUB.F64")
        {
            ss << "    @ Subtracao\n";
            ss << "    VPOP.F64 {D1}           @ POP operando B\n";
            ss << "    VPOP.F64 {D0}           @ POP operando A\n";
            ss << "    VSUB.F64 D2, D0, D1     @ A - B\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH pilha\n\n";
        }
        else if (node->opcode == "VMUL.F64")
        {
            ss << "    @ Multiplicacao\n";
            ss << "    VPOP.F64 {D1}           @ POP B\n";
            ss << "    VPOP.F64 {D0}           @ POP A\n";
            ss << "    VMUL.F64 D2, D0, D1     @ D2 = A * B\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH\n\n";
        }
        else if (node->opcode == "VDIV.F64")
        {
            ss << "    @ Divisao Real (|)\n";
            ss << "    VPOP.F64 {D1}           @ POP B\n";
            ss << "    VPOP.F64 {D0}           @ POP A\n";
            ss << "    VDIV.F64 D2, D0, D1     @ D2 = A / B\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH\n\n";
        }
        // Operadores compostos: sequencias de VFP.
        else if (node->opcode == "DIV_INT")
        {
            ss << "    @ Divisao Inteira (/)\n";
            ss << "    VPOP.F64 {D1}           @ POP B\n";
            ss << "    VPOP.F64 {D0}           @ POP A\n";
            ss << "    VDIV.F64 D2, D0, D1     @ D2 = A / B\n";
            ss << "    VCVT.S32.F64 S4, D2     @ Trunca o resultado para inteiro\n";
            ss << "    VCVT.F64.S32 D2, S4     @ Converte de volta para Double (sem decimais)\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH\n\n";
        }
        else if (node->opcode == "MOD_INT")
        {
            ss << "    @ Modulo (%)\n";
            ss << "    VPOP.F64 {D1}           @ POP B\n";
            ss << "    VPOP.F64 {D0}           @ POP A\n";
            ss << "    VDIV.F64 D2, D0, D1     @ D2 = A / B\n";
            ss << "    VCVT.S32.F64 S4, D2     @ S4 = parte inteira de (A/B)\n";
            ss << "    VCVT.F64.S32 D3, S4     @ D3 = parte inteira em Double\n";
            ss << "    VMUL.F64 D4, D1, D3     @ D4 = B * parte_inteira(A/B)\n";
            ss << "    VSUB.F64 D5, D0, D4     @ D5 = A - D4 (sobra o resto)\n";
            ss << "    VPUSH.F64 {D5}          @ PUSH Resto\n\n";
        }
        else if (node->opcode == "POW")
        {
            int id = contadorLabel++;
            ss << "    @ Potenciacao (^)\n";
            ss << "    VPOP.F64 {D1}           @ POP B (Expoente)\n";
            ss << "    VPOP.F64 {D0}           @ POP A (Base)\n";
            ss << "    VCVT.S32.F64 S2, D1     @ Converte expoente para inteiro (S2)\n";
            ss << "    VMOV R4, S2             @ Move expoente para R4 (Contador de Hardware)\n";
            ss << "    LDR R5, =LIT_ONE_POW    @ Carrega ponteiro do 1.0\n";
            ss << "    VLDR.F64 D2, [R5]       @ D2 = 1.0 (Acumulador neutro)\n";
            ss << "pow_loop_" << id << ":\n";
            ss << "    CMP R4, #0              @ Verifica se o contador e <= 0\n";
            ss << "    BLE pow_end_" << id << "   @ Quebra o laco se o expoente esgotar\n";
            ss << "    VMUL.F64 D2, D2, D0     @ Acumula: D2 = D2 * Base\n";
            ss << "    SUB R4, R4, #1          @ Decrementa contador\n";
            ss << "    B pow_loop_" << id << "    @ Retorna pro laco\n";
            ss << "pow_end_" << id << ":\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH resultado\n\n";
        }
        break;

    case ASTNodeType::INSTRUCAO_CMP:
        // logica de controle e saltos do programa VCMP.F64 + VMRS + execucao condicional dentro de IFELSE/WHILE
        // Na aritmetica funcional apenas preserva os efeitos colaterais dos operandos
        if (node->filhos.size() >= 2)
        {
            emitirNo(node->filhos[0], ss, literais, variaveis, contadorLabel);
            emitirNo(node->filhos[1], ss, literais, variaveis, contadorLabel);
        }
        break;

    case ASTNodeType::COMANDO_IFELSE:
    case ASTNodeType::COMANDO_WHILE:
    case ASTNodeType::SEQUENCIA:
        // logica de controle de emissao completa com labels proprios (contadorLabel reservado)
        // Na aritmetica funcional apenas emite os filhos para preservar efeitos (STORE/RES)
        for (ASTNode *filho : node->filhos)
            emitirNo(filho, ss, literais, variaveis, contadorLabel);
        break;
    }
}


// recebe a raiz da AST e emite Assembly ARMv7 VFP
void gerarAssembly(ASTNode *arvore, std::string &codigoAssembly)
{
    codigoAssembly.clear();
    if (arvore == nullptr)
        return;

    // Analise estatica -- coleta literais/variaveis para a secao .data.
    std::map<std::string, std::string> literais;
    std::map<std::string, std::string> variaveis;
    int contagemLiterais = 0;
    coletarLiteraisVariaveis(arvore, literais, variaveis, contagemLiterais);

    std::stringstream ss;

    // Diretivas do compilador Assembly
    ss << "    .syntax unified\n\n";

    // Secao .data: reserva de memoria RAM
    ss << "    .data\n";
    ss << "    STACK_RES: .space 8000      @ Pilha para guardar os 1000 ultimos resultados\n";
    ss << "    STACK_RES_TOP:              @ Topo da pilha\n";
    ss << "    LIT_ONE_POW: .double 1.0    @ Constante 1 para o acumulador da potenciacao\n";

    // Literais numericos lidos do arquivo .txt
    for (const auto &p : literais)
        ss << "    " << p.second << ": .double " << p.first << "\n";
    // Variaveis inicializadas em 0.0
    for (const auto &p : variaveis)
        ss << "    " << p.second << ": .double 0.0\n";

    // Secao .text: codigo executavel
    ss << "\n    .text";
    ss << "\n    .global _start\n";
    ss << "_start:\n\n";

    ss << "    @ Inicializa o Stack Pointer alinhado em 8 bytes para VFP\n";
    ss << "    LDR SP, =0x3FFFFFF8\n\n";

    ss << "    @ Inicializa o RESult stackpointer\n";
    ss << "    LDR R10, =STACK_RES_TOP\n\n";

    ss << "    @ Percorre a AST em pos-ordem (equivale a avaliacao RPN)\n";

    // Walker recursivo pos-ordem
    int contadorLabel = 0;
    emitirNo(arvore, ss, literais, variaveis, contadorLabel);

    // Trava o ultimo resultado em D15 para os LEDs
    ss << "    @ Trava o ultimo resultado do historico no D15 para os LEDs\n";
    ss << "    VLDR.F64 D15, [R10]\n\n";

    // Loop interativo de hardware: LEDs e botoes
    ss << "_interactive_loop:\n";
    ss << "    LDR R0, =0xFF200050     @ Endereco dos Botoes\n";
    ss << "    LDR R1, [R0]            @ Le botoes\n";

    // VMOV transfere os 64 bits de D15 para o par de registradores R2 e R3
    // R2 recebe a Word Baixa (bits 0-31), R3 recebe a Word Alta (bits 32-63)
    ss << "    VMOV R2, R3, D15        @ Extrai bits brutos R3=High, R2=Low\n";

    // KEY1 e KEY0 com controle de estado estrito
    ss << "    MOV R4, #0              @ Default: Apaga todos os LEDs\n";
    ss << "    TST R1, #1              @ KEY0 pressionado (Bit 0)?\n";
    ss << "    MOVNE R4, R2            @ Sim, sobrepoe R4 com a Word Baixa\n";
    ss << "    TST R1, #2              @ KEY1 pressionado (Bit 1)?\n";
    ss << "    MOVNE R4, R3            @ Sim, sobrepoe R4 com a Word Alta\n";

    // Atualiza os LEDs com o registrador escolhido
    ss << "    LDR R5, =0xFF200000     @ Endereco dos LEDs\n";
    ss << "    STR R4, [R5]            @ Atualiza os LEDs\n";
    ss << "    B _interactive_loop     @ Watchdog do loop\n";

    codigoAssembly = ss.str();
}
