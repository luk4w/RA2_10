cmake_minimum_required(VERSION 3.15)

# Define o nome do projeto, versão e linguagens utilizadas
project(asmnator VERSION 1.0 LANGUAGES C CXX)

# Inclusão de diretório(s)
include_directories(include)

# Define os arquivos de código-fonte
set(SOURCES
    src/main.cpp
    src/cli_controller.cpp
    src/fsm_scanner.cpp
    src/rpn_evaluator.cpp
    src/armv7_generator.cpp
)

# Define o nome do executável e o(s) arquivo(s) fonte
add_executable(${PROJECT_NAME} ${SOURCES})