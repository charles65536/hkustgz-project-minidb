#ifndef DISK_STORAGE_H
#define DISK_STORAGE_H

#include "database.hpp"
#include <filesystem>

class DiskStorage {
public:
    std::shared_ptr<Database> create_database(std::string name);
    std::shared_ptr<Database> load_database(std::string name);
    void save_database(Database db, std::string name);
    void delete_database(std::string name);
    std::vector<std::string> list_databases();
};


#endif
