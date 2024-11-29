
#ifndef SQL_HANDLE_H
#define SQL_HANDLE_H

#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <regex>
#include <unordered_set>
#include "expr.hpp"
#include "sql_handle.hpp"

namespace token {

class Token {
public:
    virtual ~Token() = default;
    virtual std::string str() const = 0;
};

using TokenPtr = std::shared_ptr<Token>;
using TokenList = std::vector<TokenPtr>;

class Keyword : public Token {
    std::string value;
public:
    Keyword(std::string v) : value(std::move(v)) {}
    std::string str() const override { return value; }
};

class Identifier : public Token {
    std::string value;
public:
    Identifier(std::string v) : value(std::move(v)) {}
    std::string str() const override { return value; }
};

class Literal : public Token {
    std::string value;
public:
    Literal(std::string v) : value(std::move(v)) {}
    std::string str() const override { return value; }
};

class Operator : public Token {
    std::string value;
public:
    Operator(std::string v) : value(std::move(v)) {}
    std::string str() const override { return value; }
};

class Punctuation : public Token {
    char value;
public:
    Punctuation(char v) : value(v) {}
    std::string str() const override { return std::string(1, value); }
};

} // namespace token

std::string cleanse(std::string input) {
    std::stringstream ss;
    bool in_quote = false;
    
    for (size_t i = 0; i < input.length(); i++) {
        char c = input[i];
        
        if (c == '\'') {
            in_quote = !in_quote;
            ss << c;
            continue;
        }
        
        if (in_quote) {
            if (c == ' ') ss << '_';
            else ss << c;
            continue;
        }
        
        if (std::string("()+-*/,;=<>").find(c) != std::string::npos) {
            ss << ' ' << c << ' ';
        } else {
            ss << c;
        }
    }
    
    return ss.str();
}


extern const std::unordered_set<std::string> KEYWORDS;

token::TokenList tokenize(const std::string& input) {
    token::TokenList tokens;
    std::istringstream iss(input);
    std::string token;
    
    while (iss >> token) {
        if (KEYWORDS.find(token) != KEYWORDS.end()) {
            tokens.push_back(std::make_shared<token::Keyword>(token));
            continue;
        }
        
        if (token[0] == '\'') {
            std::string literal = token.substr(1, token.length()-2);
            literal = std::regex_replace(literal, std::regex("_"), " ");
            tokens.push_back(std::make_shared<token::Literal>(literal));
            continue;
        }
        
        if (std::string("(),;").find(token[0]) != std::string::npos) {
            tokens.push_back(std::make_shared<token::Punctuation>(token[0]));
            continue;
        }
        
        if (std::string("=<>+-*/").find(token[0]) != std::string::npos) {
            tokens.push_back(std::make_shared<token::Operator>(token));
            continue;
        }
        
        if (std::isdigit(token[0]) || token[0] == '-' || token[0] == '.') {
            tokens.push_back(std::make_shared<token::Literal>(token));
            continue;
        }
        
        tokens.push_back(std::make_shared<token::Identifier>(token));
    }
    return tokens;
}

ExprPtr SqlInterpreter::read_expr() {
    auto start = cursor;
    int paren_cnt = 0;
    
    while (cursor != tokens.end()) {
        if (typeid(*peek()) == typeid(token::Punctuation)) {
            auto p = std::dynamic_pointer_cast<token::Punctuation>(peek())->str();
            if (p == "(") paren_cnt++;
            else if (p == ")") {
                paren_cnt--;
                if (paren_cnt < 0) break;
            }
        }
        cursor++;
    }
    
    auto end = cursor;
    return parse_expr_range(start, end);  // cursor already at right position
}

ExprPtr SqlInterpreter::parse_expr_range(token::TokenList::iterator start, token::TokenList::iterator end) {
    // Find top-level operator
    auto op_pos = end;
    int paren_cnt = 0;
    int min_priority = 999;
    
    for (auto it = start; it != end; ++it) {
        const token::TokenPtr& token = *it;
        
        if (typeid(*token) == typeid(token::Punctuation)) {
            auto p = std::dynamic_pointer_cast<token::Punctuation>(token)->str();
            if (p == "(") paren_cnt++;
            else if (p == ")") paren_cnt--;
        }
        else if (paren_cnt == 0 && typeid(*token) == typeid(token::Operator)) {
            auto op = std::dynamic_pointer_cast<token::Operator>(token)->str();
            int priority;
            if (op == "+" || op == "-") priority = 1;
            else if (op == "*" || op == "/") priority = 2;
            else priority = 0;  // comparison ops
            
            if (priority <= min_priority) {
                min_priority = priority;
                op_pos = it;
            }
        }
    }
    
    // No operator found - must be a single term or parenthesized expr
    if (op_pos == end) {
        if (start == end) {
            throw std::runtime_error("Empty expression");
        }
        
        const token::TokenPtr& token = *start;
        // Check if parenthesized
        if (typeid(*token) == typeid(token::Punctuation) &&
            std::dynamic_pointer_cast<token::Punctuation>(token)->str() == "(") {
            return parse_expr_range(std::next(start), std::prev(end));
        }
        
        // Must be literal/identifier
        if (typeid(*token) == typeid(token::Identifier)) {
            return col(std::dynamic_pointer_cast<token::Identifier>(token)->str());
        }
        if (typeid(*token) == typeid(token::Literal)) {
            return literal(inferred_cell(std::dynamic_pointer_cast<token::Literal>(token)->str()));
        }
        throw std::runtime_error("Invalid expression term");
    }
    
    // Build expression tree with found operator
    auto op = std::dynamic_pointer_cast<token::Operator>(*op_pos)->str();
    ExprPtr left = parse_expr_range(start, op_pos);
    ExprPtr right = parse_expr_range(std::next(op_pos), end);
    
    if (op == "+") return left + right;
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/") return left / right;
    if (op == "<") return left < right;
    if (op == ">") return left > right;
    if (op == "=") return left == right;
    throw std::runtime_error("Unknown operator: " + op);
}


#endif
