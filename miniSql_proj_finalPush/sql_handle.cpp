#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <regex>
#include <unordered_set>
#include "expr.hpp"
#include "sql_handle.hpp"




const std::unordered_set<std::string> KEYWORDS = {
    "CREATE", "DROP", "USE", "DATABASE", "TABLE",
    "INSERT", "INTO", "VALUES", "DELETE", "FROM",
    "UPDATE", "SET", "SELECT", "WHERE", "INTEGER",
    "FLOAT", "TEXT"
};

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
            if (c == ' ' || c == '\n') ss << '_';
            else ss << c;
            continue;
        }
        
        if (c == '\n') {
            ss << ' ';
        } else if (std::string("()+-*/,;=<>").find(c) != std::string::npos) {
            ss << ' ' << c << ' ';
        } else {
            ss << c;
        }
    }
    
    return ss.str();
}
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
        auto& token = *cursor;
        
        if (typeid(*token) == typeid(token::Punctuation)) {
            auto p = std::dynamic_pointer_cast<token::Punctuation>(token)->str();
            if (p == "(") {
                paren_cnt++;
            } else if (p == ")") {
                paren_cnt--;
                if (paren_cnt < 0) break;
            } else if (p == ";" || p == ",") {
                break;
            }
        } else if (paren_cnt == 0 &&
                  !(typeid(*token) == typeid(token::Identifier) ||
                    typeid(*token) == typeid(token::Literal) ||
                    typeid(*token) == typeid(token::Operator))) {
            break;
        }
        cursor++;
    }
    
    return parse_expr_range(start, cursor);
}

ExprPtr SqlInterpreter::read_condition() {
    ExprPtr condition = read_expr();
    
    if (cursor != tokens.end() && typeid(*peek()) == typeid(token::Keyword)) {
        auto op = std::dynamic_pointer_cast<token::Keyword>(peek())->str();
        if (op == "AND" || op == "OR") {
            cursor++;
            ExprPtr right = read_expr();
            return op == "AND" ? condition && right : condition || right;
        }
    }
    
    return condition;
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

// sql_handle.cpp
std::vector<CellData> SqlInterpreter::read_values() {
   std::vector<CellData> values;
   
   if (typeid(*peek()) != typeid(token::Punctuation) ||
       std::dynamic_pointer_cast<token::Punctuation>(peek())->str() != "(") {
       throw std::runtime_error("Expected ( after VALUES");
   }
   cursor++;

   while (true) {
       if (typeid(*peek()) != typeid(token::Literal)) {
           throw std::runtime_error("Expected literal in VALUES");
       }
       values.push_back(inferred_cell(std::dynamic_pointer_cast<token::Literal>(peek())->str()));
       cursor++;

       if (typeid(*peek()) == typeid(token::Punctuation)) {
           auto p = std::dynamic_pointer_cast<token::Punctuation>(peek())->str();
           if (p == ")") {
               cursor++;
               break;
           }
           if (p == ",") {
               cursor++;
               continue;
           }
       }
       throw std::runtime_error("Expected , or ) in VALUES");
   }
   
   return values;
}

Schema SqlInterpreter::read_schema() {
   if (typeid(*peek()) != typeid(token::Punctuation) ||
       std::dynamic_pointer_cast<token::Punctuation>(peek())->str() != "(") {
       throw std::runtime_error("Expected ( after CREATE TABLE");
   }
   cursor++;
   
   Schema schema;
   while (true) {
       if (typeid(*peek()) != typeid(token::Identifier)) {
           throw std::runtime_error("Expected column name");
       }
       auto col_name = std::dynamic_pointer_cast<token::Identifier>(peek())->str();
       cursor++;
       
       if (typeid(*peek()) != typeid(token::Keyword)) {
           throw std::runtime_error("Expected type after column name");
       }
       auto type_str = std::dynamic_pointer_cast<token::Keyword>(peek())->str();
       if (type_str == "INTEGER") schema[col_name] = DataType::INTEGER;
       else if (type_str == "FLOAT") schema[col_name] = DataType::FLOAT;
       else if (type_str == "TEXT") schema[col_name] = DataType::TEXT;
       else throw std::runtime_error("Unknown type: " + type_str);
       cursor++;

       if (typeid(*peek()) == typeid(token::Punctuation)) {
           auto p = std::dynamic_pointer_cast<token::Punctuation>(peek())->str();
           if (p == ")") {
               cursor++;
               break;
           }
           if (p == ",") {
               cursor++;
               continue;
           }
       }
       throw std::runtime_error("Expected , or ) in schema");
   }
   
   return schema;
}

std::vector<std::string> SqlInterpreter::read_select_list() {
   std::vector<std::string> columns;
   
   while (true) {
       if (typeid(*peek()) != typeid(token::Identifier)) {
           throw std::runtime_error("Expected column name in SELECT");
       }
       columns.push_back(std::dynamic_pointer_cast<token::Identifier>(peek())->str());
       cursor++;

       if (typeid(*peek()) == typeid(token::Punctuation) &&
           std::dynamic_pointer_cast<token::Punctuation>(peek())->str() == ",") {
           cursor++;
           continue;
       }
       break;
   }
   
   return columns;
}

NamedVector<ExprPtr> SqlInterpreter::read_set() {
   NamedVector<ExprPtr> assignments;
   
   while (true) {
       if (typeid(*peek()) != typeid(token::Identifier)) {
           throw std::runtime_error("Expected column name in SET");
       }
       auto col = std::dynamic_pointer_cast<token::Identifier>(peek())->str();
       cursor++;
       
       if (std::dynamic_pointer_cast<token::Operator>(peek())->str() != "=") {
           throw std::runtime_error("Expected = after column name");
       }
       cursor++;
       
       auto expr = read_expr();
       assignments[col] = expr;

       if (typeid(*peek()) == typeid(token::Punctuation) &&
           std::dynamic_pointer_cast<token::Punctuation>(peek())->str() == ",") {
           cursor++;
           continue;
       }
       break;
   }
   
   return assignments;
}

// sql_handle.cpp
void SqlInterpreter::execute(const std::string& sql) {
    outputTables.clear();
    tokens = tokenize(cleanse(sql));
    cursor = tokens.begin();
    
    while (cursor != tokens.end()) {
        if (typeid(*peek()) != typeid(token::Keyword)) {
            throw std::runtime_error("Expected command keyword");
        }
        
        auto cmd = std::dynamic_pointer_cast<token::Keyword>(peek())->str();
        cursor++;
        
         // the output is set to None at beginning of commands. SELECT sets it to result at the end.
        if (cmd == "CREATE") parse_create();
        else if (cmd == "USE") parse_use();
        else if (cmd == "DROP") parse_drop();
        else if (cmd == "INSERT") parse_insert();
        else if (cmd == "SELECT") parse_select();
        else if (cmd == "UPDATE") parse_update();
        else if (cmd == "DELETE") parse_delete();
        else throw std::runtime_error("Unknown command: " + cmd);
    }
}

void SqlInterpreter::parse_create() {
    try {
        auto type = read_token<token::Keyword>().str();
        if (type == "DATABASE") {
            auto name = read_token<token::Identifier>().str();
            expect(";", "Missing semicolon after CREATE DATABASE");
            
            // Just create the database, don't select it
            storage.create_database(name);
            // Remove the automatic selection I had incorrectly added before
            // current_db = storage.create_database(name);
            // current_db_name = name;
        }
        else if (type == "TABLE") {
            if (!current_db) throw std::runtime_error("No database selected");
            auto name = read_token<token::Identifier>().str();
            auto schema = read_schema();
            expect(";", "Missing semicolon after CREATE TABLE");
            current_db->create_table(name, schema);
            
            // Save database after creating new table
            storage.save_database(*current_db, current_db_name);
        }
        else throw std::runtime_error("Expected DATABASE or TABLE after CREATE");
    } catch (const std::bad_cast&) {
        throw std::runtime_error("Invalid CREATE syntax");
    }
}

void SqlInterpreter::parse_use() {
    try {
        expect("DATABASE", "Expected DATABASE after USE");
        auto name = read_token<token::Identifier>().str();
        expect(";", "Missing semicolon after USE DATABASE");
        
        // Close and save current database before loading new one
        close_database();
        
        // Load new database and store its name
        current_db = storage.load_database(name);
        current_db_name = name;
    } catch (const std::bad_cast&) {
        throw std::runtime_error("Invalid USE syntax");
    }
}

void SqlInterpreter::parse_drop() {
    try {
        auto type = read_token<token::Keyword>().str();
        if (type == "DATABASE") {
            auto name = read_token<token::Identifier>().str();
            expect(";", "Missing semicolon after DROP DATABASE");
            storage.delete_database(name);
        }
        else if (type == "TABLE") {
            if (!current_db) throw std::runtime_error("No database selected");
            auto name = read_token<token::Identifier>().str();
            expect(";", "Missing semicolon after DROP TABLE");
            current_db->drop_table(name);
        }
        else throw std::runtime_error("Expected DATABASE or TABLE after DROP");
    } catch (const std::bad_cast&) {
        throw std::runtime_error("Invalid DROP syntax");
    }
}

void SqlInterpreter::parse_insert() {
    try {
        expect("INTO", "Expected INTO after INSERT");
        auto table_name = read_token<token::Identifier>().str();
        expect("VALUES", "Expected VALUES after table name");
        auto values = read_values();
        expect(";", "Missing semicolon after INSERT");

        auto& table = current_db->get_table(table_name);
        if (values.size() != table.schema.size()) {
            throw std::runtime_error("Value count mismatch");
        }

        Row row(table.schema);
        for (size_t i = 0; i < values.size(); i++) {
            row.cells[table.schema.elements[i].name] = values[i];
        }
        table.append_row(row);
    } catch (const std::bad_cast&) {
        throw std::runtime_error("Invalid INSERT syntax");
    }
}

void SqlInterpreter::parse_select() {
    try {
        std::vector<std::string> cols;
        if (peek()->str() == "*") {
            cursor++;
        } else {
            cols = read_select_list();
        }
        
        expect("FROM", "Expected FROM after SELECT");
        auto table_name = read_token<token::Identifier>().str();
        
        // Get initial table
        auto& base_table = current_db->get_table(table_name);
        Table result = base_table;  // Start with base table
        
        // Check for WHERE or INNER JOIN or semicolon
        if (cursor != tokens.end()) {
            if (peek()->str() == "INNER") {
                cursor++;
                expect("JOIN", "Expected JOIN after INNER");
                
                // Get the table to join with
                auto join_table_name = read_token<token::Identifier>().str();
                auto& join_table = current_db->get_table(join_table_name);
                
                expect("ON", "Expected ON after INNER JOIN table");
                
                // Read the join condition
                ExprPtr join_condition = read_condition();
                
                // Perform the join and filter
                result = result.join(join_table).where(join_condition);
                
                // Now look for WHERE or semicolon
                if (cursor != tokens.end() && peek()->str() == "WHERE") {
                    cursor++;
                    ExprPtr where_condition = read_condition();
                    result = result.where(where_condition);
                }
                
                expect(";", "Missing semicolon after JOIN clause");
            }
            else if (peek()->str() == "WHERE") {
                cursor++;
                ExprPtr condition = read_condition();
                result = result.where(condition);
                expect(";", "Missing semicolon after WHERE clause");
            }
            else {
                expect(";", "Missing semicolon after FROM clause");
            }
        }
        
        // Add result to output
        if (cols.empty()) { // * case
            outputTables.push_back(result);
        } else {
            outputTables.push_back(result.select(cols));
        }
        
    } catch (const std::bad_cast&) {
        throw std::runtime_error("Invalid SELECT syntax");
    }
}

void SqlInterpreter::parse_update() {
    try {
        auto table_name = read_token<token::Identifier>().str();
        expect("SET", "Expected SET after table name");
        auto assignments = read_set();
        
        ExprPtr condition;
        if (cursor != tokens.end() && peek()->str() == "WHERE") {
            cursor++;
            condition = read_condition();
        }
        expect(";", "Missing semicolon after UPDATE");

        auto& table = current_db->get_table(table_name);
        if (condition) {
            table.update_where(condition, assignments);
        } else {
            table.update_where(literal(1), assignments);
        }
    } catch (const std::bad_cast&) {
        throw std::runtime_error("Invalid UPDATE syntax");
    }
}

void SqlInterpreter::parse_delete() {
    try {
        expect("FROM", "Expected FROM after DELETE");
        auto table_name = read_token<token::Identifier>().str();
        
        ExprPtr condition;
        if (cursor != tokens.end() && peek()->str() == "WHERE") {
            cursor++;
            condition = read_condition();
        }
        expect(";", "Missing semicolon after DELETE");

        auto& table = current_db->get_table(table_name);
        if (condition) {
            table.delete_where(condition);
        } else {
            table.delete_where(literal(1));
        }
    } catch (const std::bad_cast&) {
        throw std::runtime_error("Invalid DELETE syntax");
    }
}

void SqlInterpreter::expect(const std::string& token_expected, const std::string& error_msg) {
    if (cursor == tokens.end()) {
        throw std::runtime_error(error_msg.empty() ?
            "Unexpected end of input, expected: " + token_expected :
            error_msg);
    }
    
    if (peek()->str() != token_expected) {
        throw std::runtime_error(error_msg.empty() ?
            "Expected " + token_expected + ", found: " + peek()->str() :
            error_msg);
    }
    cursor++;
}

