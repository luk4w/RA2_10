#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <set>

std::map<std::string, std::set<std::string>> calcularFirst();
std::map<std::string, std::set<std::string>> calcularFollow();

extern std::map<std::string, std::map<std::string, std::vector<std::string>>> tabela_ll1;

void construirTabelaLL1();
void construirGramatica();