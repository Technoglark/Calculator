#pragma once

#include "calculator.h"

#include <cstdint>
#include <sstream>
#include <string>

inline int64_t EvaluateExpression(const std::string& expression) {
    std::istringstream ss{expression};
    Tokenizer tokenizer{&ss};
    auto expr = ParseExpression(&tokenizer);
    return expr->Evaluate();
}
