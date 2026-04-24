# ANALISADOR SINTÁTICO LL(1)

Lucas Franco de Mello - luk4w
Nome do grupo no Canvas: RA2 10

> Devido a plataforma do github não permitir a criação de repositórios com espaços, o nome do grupo foi alterado para RA2_10, no entanto, o nome real do grupo é RA2 10.

O `ANALISADOR SINTÁTICO` é a segundo fase do projeto da disciplina de `Linguagens Formais e Compiladores` ministrada pelo professor `Frank de Alcantara` na `Pontifícia Universidade Católica do Paraná`.

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
5. Se não houver erros, o executável final (`AnalisadorSintatico.exe`) será buildado dentro do diretório `build/Debug/` (ou `build/Release/`), dependendo da configuração selecionada.


## Gramática EBNF LL(1) Fatorada

Para garantir que o Analisador Sintático opere de forma determinística, a gramática em Notação Polonesa Reversa (RPN) foi submetida à Fatoração à Esquerda. Isso eliminou os conflitos de derivação, e resultou na seguinte estrutura formal:

**Regras de Produção:**
1. `programa -> PAREN_ESQ START PAREN_DIR sequencia_execucao`
2. `sequencia_execucao -> PAREN_ESQ avaliacao_sequencia`
3. `avaliacao_sequencia -> END PAREN_DIR | corpo_expressao PAREN_DIR sequencia_execucao`
4. `expressao_aninhada -> PAREN_ESQ corpo_expressao PAREN_DIR`
5. `operando -> NUMERO | expressao_aninhada`
6. `corpo_expressao -> operando complemento_expressao | ID`
7. `complemento_expressao -> operando operacao | ID`
8. `operacao -> OP_MAT | OP_REL | operando IFELSE | WHILE`

### Dicionário de Símbolos

**Terminais (Tokens):**
`PAREN_ESQ`, `PAREN_DIR`, `START`, `END`, `NUMERO`, `ID`, `OP_MAT`, `OP_REL`, `IFELSE`, `WHILE`

**Não-Terminais (Variáveis da AST):**
`programa`, `sequencia_execucao`, `avaliacao_sequencia`, `expressao_aninhada`, `operando`, `corpo_expressao`, `complemento_expressao`, `operacao`

### Conjuntos FIRST e FOLLOW

O cálculo teórico exigido para provar a ausência de conflitos ambíguos na árvore sintática:

**FIRST**:
* **FIRST(programa)** = { `PAREN_ESQ` }
* **FIRST(sequencia_execucao)** = { `PAREN_ESQ` }
* **FIRST(avaliacao_sequencia)** = { `END`, `NUMERO`, `PAREN_ESQ`, `ID` }
* **FIRST(expressao_aninhada)** = { `PAREN_ESQ` }
* **FIRST(operando)** = { `NUMERO`, `PAREN_ESQ` }
* **FIRST(corpo_expressao)** = { `NUMERO`, `PAREN_ESQ`, `ID` }
* **FIRST(complemento_expressao)** = { `NUMERO`, `PAREN_ESQ`, `ID` }
* **FIRST(operacao)** = { `OP_MAT`, `OP_REL`, `NUMERO`, `PAREN_ESQ`, `WHILE` }

**FOLLOW**:
* **FOLLOW(programa)** = { `$` }
* **FOLLOW(sequencia_execucao)** = { `$` }
* **FOLLOW(avaliacao_sequencia)** = { `$` }
* **FOLLOW(expressao_aninhada)** = { `NUMERO`, `PAREN_ESQ`, `ID`, `OP_MAT`, `OP_REL`, `WHILE`, `IFELSE` }
* **FOLLOW(operando)** = { `NUMERO`, `PAREN_ESQ`, `ID`, `OP_MAT`, `OP_REL`, `WHILE`, `IFELSE` }
* **FOLLOW(corpo_expressao)** = { `PAREN_DIR` }
* **FOLLOW(complemento_expressao)** = { `PAREN_DIR` }
* **FOLLOW(operacao)** = { `PAREN_DIR` }

### Tabela de Parsing LL(1)

A matriz de decisão para a implementação das funções recursivas do compilador. Os números indicam qual "Regra de Produção" acima deve ser invocada. Células em branco indicam *Syntax Error*.

| Não-Terminal | `(` | `)` | `START` | `END` | `NUMERO` | `ID` | `OP_MAT` | `OP_REL` | `IFELSE` | `WHILE` | `$` |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **programa** | 1 | | | | | | | | | | |
| **sequencia_execucao** | 2 | | | | | | | | | | |
| **avaliacao_sequencia**| 3b | | | 3a | 3b | 3b | | | | | |
| **expressao_aninhada** | 4 | | | | | | | | | | |
| **operando** | 5b | | | | 5a | | | | | | |
| **corpo_expressao** | 6a | | | | 6a | 6b | | | | | |
| **comp_expressao** | 7a | | | | 7a | 7b | | | | | |
| **operacao** | 8c | | | | 8c | | 8a | 8b | | 8d | |

**Legenda das ramificações:**
* **3a:** `END PAREN_DIR`
* **3b:** `corpo_expressao PAREN_DIR sequencia_execucao`
* **5a:** `NUMERO`
* **5b:** `expressao_aninhada`
* **6a:** `operando complemento_expressao`
* **6b:** `ID`
* **7a:** `operando operacao`
* **7b:** `ID`
* **8a:** `OP_MAT`
* **8b:** `OP_REL`
* **8c:** `operando IFELSE`
* **8d:** `WHILE`

