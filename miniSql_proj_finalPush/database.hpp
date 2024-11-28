#ifndef DATABASE_H
#define DATABASE_H

#include "table.hpp"
#include <unordered_map>

class Database {
public:
    std::unordered_map<std::string, Table> tables;
    
    Table &create_table(std::string name, Schema schema);
    Table &get_table(std::string name);
    bool has_table(std::string name);
    void drop_table(std::string name);
};
#endif
