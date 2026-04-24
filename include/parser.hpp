#pragma once
#include <vector>
#include <string>
#include <map>
#include <stack>
#include "ast.hpp"
#include "tokens.hpp"

ASTNode *parsear(const std::vector<TokenData> &tokens, const std::map<std::string, std::map<std::string, std::vector<std::string>>> &tabela_ll1);