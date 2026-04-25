# ANALISADOR SINTÁTICO LL(1)

Lucas Franco de Mello - luk4w
Nome do grupo no Canvas: RA2 10

Instituição: Pontifícia Universidade Católica do Paraná (PUCPR)  
Disciplina: Linguagens Formais e Autômatos  
Professor: Frank de Alcantara  
Ano: 2026

> Devido a plataforma do github não permitir a criação de repositórios com espaços, o nome do grupo foi alterado para RA2_10, no entanto, o nome real do grupo é RA2 10.

O `ANALISADOR SINTÁTICO` é a segunda fase do projeto da disciplina de `Linguagens Formais e Autômatos` ministrada pelo professor `Frank de Alcantara` na `Pontifícia Universidade Católica do Paraná`.

As instruções do professor determinam que o programa deve:
> 1. Implementar estruturas de controle (tomada de decisão e laços) mantendo a notação polonesa reversa;
> 2. Criar uma gramática LL(1) para a linguagem, incluindo as operações aritméticas, comandos especiais, e as novas estruturas de controle (decisão e laços);
> 3. Ler um arquivo de texto contendo o código-fonte de um programa escrito na linguagem especificada neste documento (uma expressão ou declaração por linha);
> 4. Utilizar o string de tokens (gerado pelo analisador léxico, como o da Fase 1) como entrada para o analisador sintático;
> 5. Implementar um analisador sintático descendente recursivo do tipo LL(1) para validar a estrutura do código;
> 6. Gerar a árvore sintática correspondente ao código-fonte, representando a hierarquia das expressões e comandos;
> 7. Detectar e reportar erros sintáticos de forma clara e informativa.
>
> — <cite>Prof. Frank de Alcantara</cite>

## Instruções de Compilação e Execução

A infraestrutura de *build* deste projeto é orquestrada pelo **CMake**, e garante uma compilação modular e reprodutível. Como o desenvolvimento tem como alvo principal o ambiente Windows, as instruções abaixo utilizam a *toolchain* do **MSVC** (Microsoft Visual C++) integrada ao **Visual Studio Code**.

#### Pré-requisitos do Ambiente
Certifique-se de ter os seguintes componentes instalados:
* **Compilador:** Ferramentas de Build do Visual Studio Community 2022 (ou superior) com suporte para desenvolvimento em C++23.
* **Editor:** Visual Studio Code.
* **Extensões (Visual Studio Code):**
  * `C/C++` (Microsoft)
  * `CMake Tools` (Microsoft)

#### Metodo 1: Compilação via Visual Studio Code
A extensão [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) automatiza o os comandos de configuração e construção, simplificando o processo. Siga os passos abaixo:

1. Abra a pasta raiz do projeto no Visual Studio Code (`File > Open Folder...`).
2. A extensão identificará o arquivo `CMakeLists.txt` então, através da paleta de comandos `Ctrl+Shift+p`, ou o atalho que você configurou, selecione um **Kit** de compilação, escolha a arquitetura nativa do MSVC (ex: `Visual Studio Community 2022 Release - x86_amd64`).
3. Aguarde o processo de configuração (*Configuring*) terminar. O CMake irá gerar a árvore de diretórios e o *cache* de compilação.
4. Utilize o atalho `Ctrl+Shift+p` para abrir a paleta de comandos, e execute `CMake: Build` para iniciar a compilação do projeto.
5. Se não houver erros, o executável final (`AnalisadorSintatico.exe`) será gerado dentro do diretório `build/Debug/` (ou `build/Release/`), dependendo da configuração selecionada.

### Sintaxe das Estruturas de Controle

A linguagem mantém a notação polonesa reversa (RPN) para todas as estruturas. Os operandos sempre precedem a keyword que os opera.

#### Tomada de Decisão — `IFELSE`

Requer **3 operandos** antes da keyword: condição, bloco verdadeiro e bloco falso.

```
( (condição) (bloco_verdadeiro) (bloco_falso) IFELSE )
```

**Exemplo** — se `A > B`, retorna `A + B`, senão retorna `A - B`:
```
( (A B >) (A B +) (A B -) IFELSE )
```

A condição deve ser uma expressão que resulte em valor comparável via operador relacional (`<`, `>`, `==`, `!=`, `<=`, `>=`). Ambos os blocos são obrigatórios.

#### Laço de Repetição — `WHILE`

Requer **2 operandos** antes da keyword: condição e bloco de repetição.

```
( (condição) (bloco_repeticao) WHILE )
```

**Exemplo** — enquanto `CONTADOR < 10`, soma `1` ao contador e armazena:
```
( ((CONTADOR) 10 <) (((CONTADOR) 1 +) CONTADOR) WHILE )
```

O laço avalia a condição antes de cada iteração. Quando a condição resulta em falso, a execução continua na linha seguinte ao `WHILE`.

### Comandos Especiais (Manipulação de Memória e Histórico)

A linguagem suporta operações de estado utilizando as palavras reservadas `RES` e identificadores arbitrários para variáveis (`MEM`). A sintaxe segue a notação polonesa reversa:

#### Leitura de Histórico `RES`: `(N RES)`
Retorna o valor computado `N` expressões atrás no histórico da FPU. O índice `N` deve ser um inteiro não negativo, onde `0` representa o último resultado avaliado. Exemplo:
```
(0 RES)
```
Carrega o resultado imediatamente anterior.

#### Escrita em Memória `STORE`: `(V MEM)`
Armazena o valor `V` (que pode ser um literal numérico ou o resultado de uma expressão aninhada) na variável identificada por `MEM`. Exemplos:
```
(10.5 CONTADOR)
```
```
((5 2 +) X)
```

#### Leitura de Memória `LOAD`: `(MEM)`
Carrega na pilha da FPU o valor atual armazenado na variável identificada. Se a variável não houver sido inicializada, o valor padrão retornado é `0.0`. Exemplo:
```
((CONTADOR) 1 +)
```

## Gramática EBNF LL(1) Fatorada

Para garantir que o Analisador Sintático opere de forma determinística, a gramática em Notação Polonesa Reversa (RPN) foi submetida à Fatoração à Esquerda. Isso eliminou os conflitos de derivação, e resultou na seguinte estrutura formal:

**Regras de Produção:**
1. `programa -> PARENTESE_ESQ START PARENTESE_DIR sequencia_execucao`
2. `sequencia_execucao -> PARENTESE_ESQ avaliacao_sequencia`
3. `avaliacao_sequencia -> END PARENTESE_DIR | corpo_expressao PARENTESE_DIR sequencia_execucao`
4. `expressao_aninhada -> PARENTESE_ESQ corpo_expressao PARENTESE_DIR`
5. `operando -> NUMERO | expressao_aninhada`
6. `corpo_expressao -> operando complemento_expressao | IDENTIFICADOR`
7. `complemento_expressao -> operando operacao | IDENTIFICADOR | RES | ε`
8. `operacao -> OPERADOR | OPERADOR_RELACIONAL | operando IFELSE | WHILE`

### Dicionário de Símbolos

**Terminais (Tokens):**
`PARENTESE_ESQ`, `PARENTESE_DIR`, `START`, `END`, `NUMERO`, `IDENTIFICADOR`, `OPERADOR`, `OPERADOR_RELACIONAL`, `IFELSE`, `WHILE`

**Não-Terminais (Variáveis da AST):**
`programa`, `sequencia_execucao`, `avaliacao_sequencia`, `expressao_aninhada`, `operando`, `corpo_expressao`, `complemento_expressao`, `operacao`

### Conjuntos FIRST e FOLLOW

O cálculo teórico exigido para provar a ausência de conflitos ambíguos na árvore sintática:

#### Conjuntos FIRST
* **FIRST(programa)** = { `PARENTESE_ESQ` }
* **FIRST(sequencia_execucao)** = { `PARENTESE_ESQ` }
* **FIRST(avaliacao_sequencia)** = { `END`, `IDENTIFICADOR`, `NUMERO`, `PARENTESE_ESQ` }
* **FIRST(expressao_aninhada)** = { `PARENTESE_ESQ` }
* **FIRST(operando)** = { `NUMERO`, `PARENTESE_ESQ` }
* **FIRST(corpo_expressao)** = { `IDENTIFICADOR`, `NUMERO`, `PARENTESE_ESQ` }
* **FIRST(complemento_expressao)** = { `IDENTIFICADOR`, `NUMERO`, `PARENTESE_ESQ`, `RES`, `ε` }
* **FIRST(operacao)** = { `NUMERO`, `OPERADOR`, `OPERADOR_RELACIONAL`, `PARENTESE_ESQ`, `WHILE` }

#### Conjuntos FOLLOW
* **FOLLOW(programa)** = { `$` }
* **FOLLOW(sequencia_execucao)** = { `$` }
* **FOLLOW(avaliacao_sequencia)** = { `$` }
* **FOLLOW(expressao_aninhada)** = { `IDENTIFICADOR`, `IFELSE`, `NUMERO`, `OPERADOR`, `OPERADOR_RELACIONAL`, `PARENTESE_ESQ`, `RES`, `WHILE` }
* **FOLLOW(operando)** = { `IDENTIFICADOR`, `IFELSE`, `NUMERO`, `OPERADOR`, `OPERADOR_RELACIONAL`, `PARENTESE_ESQ`, `RES`, `WHILE` }
* **FOLLOW(corpo_expressao)** = { `PARENTESE_DIR` }
* **FOLLOW(complemento_expressao)** = { `PARENTESE_DIR` }
* **FOLLOW(operacao)** = { `PARENTESE_DIR` }

### Tabela de Parsing LL(1)

A tabela LL(1) serve para guiar o parser preditivo descendente na determinação de qual regra gramatical (produção) deve ser aplicada a seguir, sem precisar de backtracking.

Células vazias indicam erro sintático. $\epsilon$ indica derivação para vazio.

#### Formato compacto
`M[Não-terminal, Terminal] = Produção`
```
M[avaliacao_sequencia, END] = { END PARENTESE_DIR }
M[avaliacao_sequencia, IDENTIFICADOR] = { corpo_expressao PARENTESE_DIR sequencia_execucao }
M[avaliacao_sequencia, NUMERO] = { corpo_expressao PARENTESE_DIR sequencia_execucao }
M[avaliacao_sequencia, PARENTESE_ESQ] = { corpo_expressao PARENTESE_DIR sequencia_execucao }
M[complemento_expressao, IDENTIFICADOR] = { IDENTIFICADOR }
M[complemento_expressao, NUMERO] = { operando operacao }
M[complemento_expressao, PARENTESE_DIR] = { EPSILON }
M[complemento_expressao, PARENTESE_ESQ] = { operando operacao }
M[complemento_expressao, RES] = { RES }
M[corpo_expressao, IDENTIFICADOR] = { IDENTIFICADOR }
M[corpo_expressao, NUMERO] = { operando complemento_expressao }
M[corpo_expressao, PARENTESE_ESQ] = { operando complemento_expressao }
M[expressao_aninhada, PARENTESE_ESQ] = { PARENTESE_ESQ corpo_expressao PARENTESE_DIR }
M[operacao, NUMERO] = { operando IFELSE }
M[operacao, OPERADOR] = { OPERADOR }
M[operacao, OPERADOR_RELACIONAL] = { OPERADOR_RELACIONAL }
M[operacao, PARENTESE_ESQ] = { operando IFELSE }
M[operacao, WHILE] = { WHILE }
M[operando, NUMERO] = { NUMERO }
M[operando, PARENTESE_ESQ] = { expressao_aninhada }
M[programa, PARENTESE_ESQ] = { PARENTESE_ESQ START PARENTESE_DIR sequencia_execucao }
M[sequencia_execucao, PARENTESE_ESQ] = { PARENTESE_ESQ avaliacao_sequencia }
```
 
#### Formato tabular
 
| Não-Terminal | `(` | `)` | `START` | `END` | `NUMERO` | `ID` | `OPERADOR` | `OP_REL` | `IFELSE` | `WHILE` | `RES` |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **programa** | 1 | | | | | | | | | | |
| **sequencia_execucao** | 2 | | | | | | | | | | |
| **avaliacao_sequencia** | 3b | | | 3a | 3b | 3b | | | | | |
| **expressao_aninhada** | 4 | | | | | | | | | | |
| **operando** | 5b | | | | 5a | | | | | | |
| **corpo_expressao** | 6a | | | | 6a | 6b | | | | | |
| **complemento_expressao** | 7a | 7d | | | 7a | 7b | | | | | 7c |
| **operacao** | 8c | | | | 8c | | 8a | 8b | | 8d | |
 
#### Legenda
| Código | Produção |
| :--- | :--- |
| **1** | `PARENTESE_ESQ START PARENTESE_DIR sequencia_execucao` |
| **2** | `PARENTESE_ESQ avaliacao_sequencia` |
| **3a** | `END PARENTESE_DIR` |
| **3b** | `corpo_expressao PARENTESE_DIR sequencia_execucao` |
| **4** | `PARENTESE_ESQ corpo_expressao PARENTESE_DIR` |
| **5a** | `NUMERO` |
| **5b** | `expressao_aninhada` |
| **6a** | `operando complemento_expressao` |
| **6b** | `IDENTIFICADOR` |
| **7a** | `operando operacao` |
| **7b** | `IDENTIFICADOR` |
| **7c** | `RES` |
| **7d** | `ε` |
| **8a** | `OPERADOR` |
| **8b** | `OPERADOR_RELACIONAL` |
| **8c** | `operando IFELSE` |
| **8d** | `WHILE` |


## Resultados

Para assegurar a conformidade com a norma IEEE 754 de 64 bits (Double Precision) exigida pelo edital, todos os resultados obtidos nos 32 LEDs do simulador foram submetidos a um processo de verificação cruzada.

#### Metodologia de Verificação

* **Cálculo Analítico:** Resolução manual da pilha RPN para determinar o valor decimal esperado de cada expressão.
* **Extração de Bits:** Captura dos estados lógicos dos LEDs (Palavra Alta e Palavra Baixa) a partir da injeção de JavaScript no DOM do CPULator. A montagem das palavras de 64 bits foi validada e comparada com a ferramenta online [BinaryConvert (Double Precision)](https://www.binaryconvert.com/result_double.html).

**Script de Captura (Bit Sniffer):**
```javascript
(function() {
    const leds = document.querySelectorAll('#devff200000 .dev_led_led');
    let binary = Array.from(leds)
        .map(led => led.classList.contains('dev_led_on') ? '1' : '0')
        .join('');
    let hex = parseInt(binary, 2).toString(16).toUpperCase().padStart(8, '0');
    console.log("%c ASMNATOR - Bit Sniffer ", "background: #222; color: #bada55; padding: 2px;");
    console.log("Binário (31-0): " + binary);
    console.log("Hexadecimal: 0x" + hex);
})();
```

#### Evidências de Teste (Casos de Sucesso)

A tabela abaixo detalha os testes realizados para validar a recursividade da gramática e a corretude do gerador de código para fluxos de controle e aritmética complexa.

| Caso de Teste | Expressão RPN Relevante | Resultado Esperado | Palavra Alta (Hex) | Palavra Baixa (Hex) | Status |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **Potência Recursiva (T2)** | `( ( ( ( 2 2 ^ ) 2 ^ ) 2 ^ ) )` | `256.0` | `0x40700000` | `0x00000000` |  OK |
| **Lógica Condicional (T3)** | `( ((0 RES) 500.0 >=) (512.0) (0.0) IFELSE )` | `512.0` | `0x40800000` | `0x00000000` |  OK |
| **Integração Final (T4)** | `( ((CONTADOR) 5 ==) (12 SAIDA) (2026 SAIDA) IFELSE )` | `12.0` | `0x40280000` | `0x00000000` |  OK |

#### Análise de Precisão IEEE 754

Tomando como exemplo o teste de **Lógica Condicional (T3)**, o valor decimal `512.0` é representado em hexadecimal de precisão dupla como `0x4080000000000000`. 
1. Ao pressionar **KEY1**, o simulador exibe a Palavra Alta: `0x40800000`.
2. Ao pressionar **KEY0**, o simulador exibe a Palavra Baixa: `0x00000000`.

A coincidência bit-a-bit entre o cálculo teórico e a saída do hardware valida a implementação da FPU e a sincronização de flags via `VMRS`.