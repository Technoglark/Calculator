#pragma once

#include <cctype>
#include <cstdint>
#include <istream>
#include <memory>
#include <utility>

class Tokenizer {
   public:
    explicit Tokenizer(std::istream* in) : in_{in} {
        Consume();
    }

    enum TokenType : uint8_t { kNumber, kSymbol, kEnd };

    void Consume() {
        char val = 0;
        in_->get(val);
        while (std::isspace(val) != 0) {
            in_->get(val);
        }

        if (in_->eof()) {
            type_ = kEnd;
            return;
        }

        if (std::isdigit(val) != 0) {
            in_->putback(val);
            *in_ >> number_;
            type_ = kNumber;
        } else {
            symbol_ = val;
            type_ = kSymbol;
        }
    }

    [[nodiscard]] TokenType GetType() const {
        return type_;
    }

    [[nodiscard]] int64_t GetNumber() const {
        return number_;
    }

    [[nodiscard]] char GetSymbol() const {
        return symbol_;
    }

   private:
    std::istream* in_;

    TokenType type_ = kEnd;
    int64_t number_{};
    char symbol_{};
};

class Expression {
   public:
    Expression() = default;
    Expression(Expression& other) = default;
    Expression(Expression&& other) = default;
    Expression& operator=(const Expression& other) = default;
    Expression& operator=(Expression&& other) = default;
    virtual ~Expression() = default;
    [[nodiscard]] virtual int64_t Evaluate() const = 0;
};

class Constant : public Expression {
   public:
    explicit Constant(const int64_t value) : value_(value) {
    }

    [[nodiscard]] int64_t Evaluate() const override {
        return value_;
    }

   private:
    int64_t value_ = 0;
};

class BinOperation : public Expression {
   public:
    explicit BinOperation(
        std::unique_ptr<Expression> left, std::unique_ptr<Expression> right, const char operation)
        : left_(std::move(left)), right_(std::move(right)), operation_(operation) {
    }

    [[nodiscard]] int64_t Evaluate() const override {
        switch (operation_) {
            case ('+'):
                return left_->Evaluate() + right_->Evaluate();
            case ('-'):
                return left_->Evaluate() - right_->Evaluate();
            case ('*'):
                return left_->Evaluate() * right_->Evaluate();
            case ('/'):
                return left_->Evaluate() / right_->Evaluate();
            default:
                return 0;
        }
    }

   private:
    std::unique_ptr<Expression> left_ = nullptr;
    std::unique_ptr<Expression> right_ = nullptr;
    char operation_{};
};

class UnaryOperation : public Expression {
   public:
    explicit UnaryOperation(std::unique_ptr<Expression> right) : right_(std::move(right)) {};

    [[nodiscard]] int64_t Evaluate() const override {
        return right_->Evaluate() * (-1);
    }

   private:
    std::unique_ptr<Expression> right_ = nullptr;
};

std::unique_ptr<Expression> ParseExpression(Tokenizer* token);

inline std::unique_ptr<Expression> Multiplicate(Tokenizer* token) {
    if (token->GetType() == Tokenizer::kNumber) {
        auto result = std::make_unique<Constant>(token->GetNumber());
        token->Consume();
        return result;
    }
    if (token->GetType() == Tokenizer::kSymbol && token->GetSymbol() == '(') {
        token->Consume();
        auto result = ParseExpression(token);
        token->Consume();
        return result;
    }
    if (token->GetType() == Tokenizer::kSymbol && token->GetSymbol() == '-') {
        token->Consume();
        auto result = std::make_unique<UnaryOperation>(Multiplicate(token));
        return result;
    }
    return nullptr;
}

inline std::unique_ptr<Expression> Item(Tokenizer* token) {
    auto ret = Multiplicate(token);
    while (token->GetType() == Tokenizer::kSymbol &&
           (token->GetSymbol() == '*' || token->GetSymbol() == '/')) {
        const char operation = token->GetSymbol();
        token->Consume();
        auto ret1 = Multiplicate(token);
        ret = std::make_unique<BinOperation>(std::move(ret), std::move(ret1), operation);
    }
    return ret;
}

inline std::unique_ptr<Expression> ParseExpression(Tokenizer* token) {
    auto ret = Item(token);
    while (token->GetType() == Tokenizer::kSymbol &&
           (token->GetSymbol() == '+' || token->GetSymbol() == '-')) {
        const char op = token->GetSymbol();
        token->Consume();
        auto ret1 = Item(token);
        ret = std::make_unique<BinOperation>(std::move(ret), std::move(ret1), op);
    }
    return ret;
}
