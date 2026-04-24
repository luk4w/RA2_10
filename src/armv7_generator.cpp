#include "armv7_generator.hpp"
#include <sstream>
#include <map>
#include <string>

int gerarAssembly(const std::vector<std::string> &_tokens_, std::string &codigoAssembly)
{
    std::stringstream ss;
    std::map<std::string, std::string> literais;
    std::map<std::string, std::string> variaveis;
    int contagemLiterais = 0;

    // Analise estatica: obtem o que vai gastar memoria RAM antes de executar
    for (const auto &t : _tokens_)
    {
        // Ignora sintaxe e operacoes
        if (t == "(" || t == ")" || t == "+" || t == "-" || t == "*" || t == "/" || t == "//" || t == "%" || t == "RES")
            continue;

        // Se for numero salva como literal (constante)
        if (t[0] >= '0' && t[0] <= '9')
        {
            if (literais.find(t) == literais.end())
            {
                literais[t] = "LIT_" + std::to_string(contagemLiterais++);
            }
        }
        // Se for letra maiuscula salva como variavel
        else if (t[0] >= 'A' && t[0] <= 'Z')
        {
            if (variaveis.find(t) == variaveis.end())
            {
                variaveis[t] = "VAR_" + t;
            }
        }
    }

    // Diretivas/instrucoes para o compilador assembly do simulador
    ss << "    .syntax unified\n\n";

    // Reserva espaco na memoria RAM (.data)
    ss << "    .data\n";
    ss << "    STACK_RES: .space 8000      @ Pilha para guardar os 1000 ultimos resultados\n";
    ss << "    STACK_RES_TOP:              @ Topo da pilha\n";
    ss << "    LIT_ONE_POW: .double 1.0    @ Constante 1 para o acumulador da potenciacao\n";

    // Injeta os valores dos numeros lidos do .txt
    for (const auto &p : literais)
    {
        ss << "    " << p.second << ": .double " << p.first << "\n";
    }
    // Inicia todas as variaveis com 0.0
    for (const auto &p : variaveis)
    {
        ss << "    " << p.second << ": .double 0.0\n";
    }

    //  codigo de maquina que o processador vai executar
    ss << "\n    .text";
    ss << "\n    .global _start\n";
    ss << "_start:\n\n";

    // Inicializar a Pilha do Sistema
    ss << "    @ Inicializa o Stack Pointer alinhado em 8 bytes para VFP\n";
    ss << "    LDR SP, =0x3FFFFFF8\n\n";

    // Define o registrador R10 como ponteiro da pilha de historico (RES)
    ss << "    @ Inicializa o RESult stackpointer\n";
    ss << "    LDR R10, =STACK_RES_TOP\n\n";

    // Variaveis de controle do pipeline de geracao (tempo de compilacao)
    int paren_count = 0;
    int simulated_stack = 0;

    ss << "    @ Processa a Notacao Polonesa Reversa (RPN)\n";
    for (size_t i = 0; i < _tokens_.size(); i++)
    {
        const std::string &t = _tokens_[i];

        if (t == "(")
        {
            paren_count++;
            continue;
        }
        else if (t == ")")
        {
            paren_count--;
            // Ao fechar a expressao mais externa, processa a gravacao do historico (RES)
            if (paren_count == 0 && simulated_stack > 0)
            {
                ss << "    @ salva o resultado no historico de memoria\n";
                ss << "    VPOP.F64 {D0}           @ POO resultad final da FPU\n";
                ss << "    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico\n";
                ss << "    VSTR.F64 D0, [R10]      @ Grava na RAM\n\n";
                simulated_stack--; // retira da pilha FPU simulada (tempo de compilacao)
            }
            continue;
        }

        if (t == "*")
        {
            simulated_stack--; // consome 2 op e gera 1 resultado
            ss << "    @ Multiplicacao\n";
            ss << "    VPOP.F64 {D1}           @ POP B\n";
            ss << "    VPOP.F64 {D0}           @ POP A\n";
            ss << "    VMUL.F64 D2, D0, D1     @ D2 = A * B\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH\n\n";
        }
        else if (t == "/")
        {
            simulated_stack--; // consome 2 op e gera 1 resultado
            ss << "    @ Divisao Real\n";
            ss << "    VPOP.F64 {D1}           @ POP B\n";
            ss << "    VPOP.F64 {D0}           @ POP A\n";
            ss << "    VDIV.F64 D2, D0, D1     @ D2 = A / B\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH\n\n";
        }
        else if (t == "//")
        {
            simulated_stack--; // consome 2 op e gera 1 resultado
            ss << "    @ Divisao Inteira (//)\n";
            ss << "    VPOP.F64 {D1}           @ POP B\n";
            ss << "    VPOP.F64 {D0}           @ POP A\n";
            ss << "    VDIV.F64 D2, D0, D1     @ D2 = A / B\n";
            ss << "    VCVT.S32.F64 S4, D2     @ Trunca o resultado para inteiro\n";
            ss << "    VCVT.F64.S32 D2, S4     @ Converte de volta para Double (sem decimais)\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH\n\n";
        }
        else if (t == "%")
        {
            simulated_stack--; // consome 2 op e gera 1 resultado
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
        else if (t == "^")
        {
            simulated_stack--; // consome 2 op e gera 1 resultado
            ss << "    @ Potenciacao (^)\n";
            ss << "    VPOP.F64 {D1}           @ POP B (Expoente)\n";
            ss << "    VPOP.F64 {D0}           @ POP A (Base)\n";
            ss << "    VCVT.S32.F64 S2, D1     @ Converte expoente para inteiro (S2)\n";
            ss << "    VMOV R4, S2             @ Move expoente para R4 (Contador de Hardware)\n";
            ss << "    LDR R5, =LIT_ONE_POW    @ Carrega ponteiro do 1.0\n";
            ss << "    VLDR.F64 D2, [R5]       @ D2 = 1.0 (Acumulador neutro)\n";
            // indice do for para evitar duplicidade de label
            ss << "pow_loop_" << i << ":\n";
            ss << "    CMP R4, #0              @ Verifica se o contador e <= 0\n";
            ss << "    BLE pow_end_" << i << "   @ Quebra o laco se o expoente esgotar\n";
            ss << "    VMUL.F64 D2, D2, D0     @ Acumula: D2 = D2 * Base\n";
            ss << "    SUB R4, R4, #1          @ Decrementa contador\n";
            ss << "    B pow_loop_" << i << "    @ Retorna pro laco\n";
            ss << "pow_end_" << i << ":\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH resultado\n\n";
        }
        else if (t == "+")
        {
            simulated_stack--; // consome 2 op e gera 1 resultado
            ss << "    @ Soma\n";
            ss << "    VPOP.F64 {D1}           @ POP operando B\n";
            ss << "    VPOP.F64 {D0}           @ POP operando A\n";
            ss << "    VADD.F64 D2, D0, D1     @ A + B\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH pilha\n\n";
        }
        else if (t == "-")
        {
            simulated_stack--; // consome 2 op e gera 1 resultado
            ss << "    @ Subtracao\n";
            ss << "    VPOP.F64 {D1}           @ POP operando B\n";
            ss << "    VPOP.F64 {D0}           @ POP operando A\n";
            ss << "    VSUB.F64 D2, D0, D1     @ A - B\n";
            ss << "    VPUSH.F64 {D2}          @ PUSH pilha\n\n";
        }
        else if (t == "RES")
        {
            // simulated_stack--; // consome 1 op e gera 1 resultado == 0
            // Resolve a aritmetica de ponteiros para buscar resultados antgos
            ss << "    @ Operador RES: Busca historico na memoria\n";
            ss << "    VPOP.F64 {D0}           @ Pega o N da pilha\n";
            ss << "    VCVT.S32.F64 S0, D0     @ Converte N de Double para Inteiro\n";
            ss << "    VMOV R1, S0             @ MOV N pro R1\n";

            ss << "    LSL R1, R1, #3          @ Multiplica N por 8 bytes (descobrir o offset)\n";
            ss << "    ADD R2, R10, R1         @ Soma o offset com o R10 para achar o endereco antigo\n";

            ss << "    VLDR.F64 D1, [R2]       @ Le o historico da RAM\n";
            ss << "    VPUSH.F64 {D1}          @ PUSH pilha\n\n";
        }
        else if (t[0] >= '0' && t[0] <= '9')
        {
            simulated_stack++; // Empilha constante +1
            ss << "    @ Empilha Constante: " << t << "\n";
            ss << "    LDR R0, =" << literais[t] << "\n";
            ss << "    VLDR.F64 D0, [R0]\n";
            ss << "    VPUSH.F64 {D0}\n\n";
        }
        else if (t[0] >= 'A' && t[0] <= 'Z')
        {
            // olha pra tras para verificar se a variavel tinha um parenteses colado antes dela
            // (MEM)
            if (i > 0 && _tokens_[i - 1] == "(")
            {
                simulated_stack++;
                ss << "    @ LOAD: Le a variavel " << t << " da memoria e empilha\n";
                ss << "    LDR R0, =" << variaveis[t] << "\n";
                ss << "    VLDR.F64 D0, [R0]\n";
                ss << "    VPUSH.F64 {D0}\n\n";
            }
            // Se nao tinha parasentes, significa que tem um resultado pronto esperando para ser gravado na variavel
            // (V MEM)
            else
            {
                simulated_stack--; // consome o resultado da pilha para gravar na variavel
                ss << "    @ Grava resultado na variavel " << t << "\n";
                ss << "    VPOP.F64 {D0}\n";
                ss << "    LDR R0, =" << variaveis[t] << "\n";
                ss << "    VSTR.F64 D0, [R0]\n\n";
            }
        }
    }

    // Salva o resultado no historico e trava no D15 para os botoes
    ss << "    @ Trava o ultimo resultado do historico no D15 para os LEDs\n";
    ss << "    VLDR.F64 D15, [R10]\n\n";

    // LOOP DE HARDWARE > RAW BITS IEEE 754
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
    return 0;
}