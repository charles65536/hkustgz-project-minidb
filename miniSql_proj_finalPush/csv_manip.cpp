#include "csv_manip.hpp"

std::vector<std::string> split_csv(std::string line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while(std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

std::string datatype_to_string(DataType type) {
    switch(type) {
        case DataType::INTEGER: return "INTEGER";
        case DataType::FLOAT: return "FLOAT";
        default: return "TEXT";
    }
}

DataType string_to_datatype(std::string type) {
    if(type == "INTEGER") return DataType::INTEGER;
    if(type == "FLOAT") return DataType::FLOAT;
    return DataType::TEXT;
}

std::string csv_dumps(Table table) {
    std::ostringstream ss;
    
    // Headers
    for(size_t i = 0; i < table.schema.elements.size(); i++) {
        if(i > 0) ss << ',';
        ss << table.schema.elements[i].name;
    }
    ss << '\n';
    
    // Types
    for(size_t i = 0; i < table.schema.elements.size(); i++) {
        if(i > 0) ss << ',';
        ss << datatype_to_string(table.schema[i]);
    }
    ss << '\n';
    
    // Data
    for(auto& row : table.rows) {
        for(size_t i = 0; i < row.cells.elements.size(); i++) {
            if(i > 0) ss << ',';
            ss << std::string(row.cells[i]);
        }
        ss << '\n';
    }
    return ss.str();
}

Table csv_loads(std::string csv_str, std::string table_name) {
    std::stringstream ss(csv_str);
    std::string line;
    
    std::getline(ss, line);
    auto headers = split_csv(line);
    
    std::getline(ss, line);
    auto types = split_csv(line);
    
    Schema schema;
    for(size_t i = 0; i < headers.size(); i++) {
        schema[headers[i]] = string_to_datatype(types[i]);
    }
    
    Table table(table_name, schema);
    
    while(std::getline(ss, line)) {
        if(line.empty()) continue;
        auto fields = split_csv(line);
        Row row(schema);
        for(size_t i = 0; i < fields.size(); i++) {
            row.cells[headers[i]] = CellData(schema[headers[i]], fields[i]);
        }
        table.append_row(row);
    }
    return table;
}

void csv_dump(Table table, std::string filepath) {
    std::ofstream(filepath) << csv_dumps(table);
}

Table csv_load(std::string filepath, std::string table_name) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return csv_loads(buffer.str(), table_name);
}
