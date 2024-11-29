#include "disk_storage.hpp"
#include "csv_manip.hpp"
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;
void DiskStorage::save_database(Database db, std::string name) {
    fs::path db_path = fs::path("./dbs") / name;
    fs::create_directories(db_path);
    
    for (auto& pair : db.tables) {
        std::cout << "Saving table " << pair.first << " with " << pair.second.rows.size() << " rows\n";
        std::string table_path = (db_path / (pair.first + ".csv")).string();
        csv_dump(pair.second, table_path, true);
    }
}
/*
void DiskStorage::save_database(Database db, std::string name) {
    fs::path db_path = fs::path("./dbs") / name;
    fs::create_directories(db_path);
    
    for (auto pair : db.tables) {
        std::string table_path = (db_path / (pair.first + ".csv")).string();
        csv_dump(pair.second, table_path, true);
    }
}
 */

// disk_storage.cpp
std::shared_ptr<Database> DiskStorage::create_database(std::string name) {
    fs::path db_path = fs::path("./dbs") / name;
    fs::create_directories(db_path);
    return std::make_shared<Database>();
}

std::shared_ptr<Database> DiskStorage::load_database(std::string name) {
    fs::path db_path = fs::path("./dbs") / name;
    
    if (!fs::exists(db_path)) {
        throw std::runtime_error("Database not found: " + name);
    }
    
    auto db = std::make_shared<Database>();
    for (auto entry : fs::directory_iterator(db_path)) {
        if (entry.path().extension() == ".csv") {
            std::string table_name = entry.path().stem().string();
            db->tables[table_name] = csv_load(entry.path().string(), table_name);
        }
    }
    return db;
}

void DiskStorage::delete_database(std::string name) {
    fs::path db_path = fs::path("./dbs") / name;
    if (!fs::exists(db_path)) {
        throw std::runtime_error("Database not found: " + name);
    }
    fs::remove_all(db_path);
}

std::vector<std::string> DiskStorage::list_databases() {
    std::vector<std::string> databases;
    fs::path dbs_path = "./dbs";
    fs::create_directories(dbs_path);
    
    for (auto entry : fs::directory_iterator(dbs_path)) {
        if (entry.is_directory()) {
            databases.push_back(entry.path().filename().string());
        }
    }
    return databases;
}
