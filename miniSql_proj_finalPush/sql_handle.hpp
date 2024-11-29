#ifndef SQL_HANDLE_H
#define SQL_HANDLE_H

#include "expr.hpp"
#include "schema.hpp"
#include "named_vector.hpp"
#include "table.hpp"
#include "database.hpp"
#include "disk_storage.hpp"
#include <vector>
#include <memory>
#include <sstream>
#include <regex>
#include <unordered_set>

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
}

std::string cleanse(std::string input);
token::TokenList tokenize(const std::string& input);
extern const std::unordered_set<std::string> KEYWORDS;

class SqlInterpreter {
public:
    token::TokenList tokens;
    token::TokenList::iterator cursor;
    std::shared_ptr<Database> current_db;
    std::ofstream* output = nullptr;
    DiskStorage storage;
    
    const token::TokenPtr& peek();
    
    template<typename T>
    T read_token();

    void expect(const std::string& str);
    
    // Statement parsers
    void execute(const std::string& sql);
    void execute_file(const std::string& filepath);
    void parse_create();
    void parse_use();
    void parse_drop();
    void parse_insert();
    void parse_select();
    void parse_update();
    void parse_delete();
    
    // Expression and clause parsing
    ExprPtr read_expr();
    ExprPtr parse_expr_range(token::TokenList::iterator start, token::TokenList::iterator end);
    ExprPtr read_condition();
    Schema read_schema();
    std::vector<std::string> read_select_list();
    std::vector<CellData> read_values();
    NamedVector<ExprPtr> read_set();

    // Output handling
    void set_output(std::ofstream& out);
    void output_table(const Table& table);
    void output_csv_row(const Row& row, const std::vector<std::string>& cols);
};

#endif
