#include "database.hpp"

Table Database::create_table(std::string name, Schema schema) {
    if (tables.find(name) != tables.end()) {
        throw std::runtime_error("Table already exists: " + name);
    }
    tables[name] = Table(name, schema);
    return tables[name];
}

Table Database::get_table(std::string name) {
    auto it = tables.find(name);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + name);
    }
    return it->second;
}

bool Database::has_table(std::string name) {
    return tables.find(name) != tables.end();
}

void Database::drop_table(std::string name) {
    if (!has_table(name)) {
        throw std::runtime_error("Table not found: " + name);
    }
    tables.erase(name);
}
