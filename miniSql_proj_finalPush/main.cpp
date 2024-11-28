#include "disk_storage.hpp"
#include <iostream>

void print_table_info(Table& table) {
    std::cout << "Table " << table.name << " has " << table.rows.size() << " rows\n";
}

int main() {
    DiskStorage storage;
    Database db;
    
    Schema user_schema;
    user_schema["id"] = DataType::INTEGER;
    user_schema["name"] = DataType::TEXT;
    user_schema["balance"] = DataType::FLOAT;
    auto& users = db.create_table("users", user_schema);
    
    Row u1(user_schema);
    u1.cells["id"] = CellData(1);
    u1.cells["name"] = CellData("Alice");
    u1.cells["balance"] = CellData(1000.50);
    users.append_row(u1);
    
    Row u2(user_schema);
    u2.cells["id"] = CellData(2);
    u2.cells["name"] = CellData("Bob");
    u2.cells["balance"] = CellData(2500.75);
    users.append_row(u2);
    
    
        std::cout << "After create: " << users.rows.size() << " rows\n";
        
        users.append_row(u1);
        std::cout << "After append 1: " << users.rows.size() << " rows\n";
        
        users.append_row(u2);
        std::cout << "After append 2: " << users.rows.size() << " rows\n";
        
        std::cout << "Table in db before save: " << db.tables["users"].rows.size() << " rows\n";
        storage.save_database(db, "testdb");
}
