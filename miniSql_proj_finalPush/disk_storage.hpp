#ifndef DISK_STORAGE_H
#define DISK_STORAGE_H

#include "database.hpp"
#include <filesystem>

class DiskStorage {
public:
    void save_database(Database db, std::string name);
    Database load_database(std::string name);
    void delete_database(std::string name);
    std::vector<std::string> list_databases();
};


#endif
