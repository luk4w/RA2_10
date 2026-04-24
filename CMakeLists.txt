cmake_minimum_required(VERSION 3.15)

# Define o nome do projeto, versão e linguagens utilizadas
project(AnalisadorSintatico VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Inclusão de diretório(s)
include_directories(include)

# Define os arquivos de código-fonte
set(SOURCES
    src/main.cpp
    src/cli_controller.cpp
    src/fsm_scanner.cpp
    src/armv7_generator.cpp
    src/gramatica.cpp
    src/parser.cpp
)

# Define o nome do executável e o(s) arquivo(s) fonte
add_executable(${PROJECT_NAME} ${SOURCES})