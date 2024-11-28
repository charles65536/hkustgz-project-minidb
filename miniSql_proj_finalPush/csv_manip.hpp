// csv_manip.hpp
#ifndef CSV_MANIP_H
#define CSV_MANIP_H

#include "table.hpp"
#include <fstream>
#include <sstream>

std::vector<std::string> split_csv(std::string line);
std::string datatype_to_string(DataType type);
DataType string_to_datatype(std::string type);
std::string csv_dumps(Table table);
Table csv_loads(std::string csv_str, std::string table_name);
void csv_dump(Table table, std::string filepath);
Table csv_load(std::string filepath, std::string table_name);

#endif
