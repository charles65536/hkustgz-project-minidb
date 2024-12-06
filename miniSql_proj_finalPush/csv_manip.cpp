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

// csv_manip.cpp
std::string csv_dumps(Table table, bool with_type_info, bool quoted_strs) {
    std::ostringstream ss;
    ss.precision(2);
    ss << std::fixed;
    
    // Headers
    for(size_t i = 0; i < table.schema.elements.size(); i++) {
        if(i > 0) ss << ',';
        ss << table.schema.elements[i].name;
    }
    ss << '\n';
    
    // Types (only if with_type_info is true)
    if (with_type_info) {
        for(size_t i = 0; i < table.schema.elements.size(); i++) {
            if(i > 0) ss << ',';
            ss << datatype_to_string(table.schema[i]);
        }
        ss << '\n';
    }
    
    // Data
    for(auto &row : table.rows) {
        for(size_t i = 0; i < row.cells.elements.size(); i++) {
            if(i > 0) ss << ',';
            const auto& cell = row.cells[i];
            if (cell.type == DataType::TEXT && quoted_strs) {
                ss << "\'" << std::string(cell) << "\'";
            } else if (cell.type == DataType::FLOAT) {
                ss << double(cell);
            } else {
                ss << std::string(cell);
            }
        }
        ss << '\n';
    }
    return ss.str();
}

Table csv_loads(std::string csv_str, std::string table_name, bool with_type_info, bool quoted_strs) {
    std::stringstream ss(csv_str);
    std::string line;
    
    std::getline(ss, line);
    auto headers = split_csv(line);
    
    // Try to read types if present
    std::getline(ss, line);
    auto types = split_csv(line);
    bool has_types = true;
    for(auto type : types) {
        if(type != "INTEGER" && type != "FLOAT" && type != "TEXT") {
            has_types = false;
            break;
        }
    }
    
    Schema schema;
    if(has_types) {
        for(size_t i = 0; i < headers.size(); i++) {
            schema[headers[i]] = string_to_datatype(types[i]);
        }
    } else {
        // If no type info, default to TEXT
        for(auto header : headers) {
            schema[header] = DataType::TEXT;
        }
        // Reset stream position to process the line as data
        ss.clear();
        ss.seekg(0);
        std::getline(ss, line); // Skip header
    }
    
    Table table(table_name, schema);
    
    while(std::getline(ss, line)) {
        if(line.empty()) continue;
        auto fields = split_csv(line);
        Row row(schema);
        for(size_t i = 0; i < fields.size(); i++) {
            std::string field = fields[i];
            // Handle quoted strings if needed
            if (quoted_strs && schema[headers[i]] == DataType::TEXT) {
                if (field.size() >= 2 && field.front() == '"' && field.back() == '"') {
                    field = field.substr(1, field.size() - 2);
                }
            }
            row.cells[headers[i]] = CellData(schema[headers[i]], field);
        }
        table.append_row(row);
    }
    return table;
}

void csv_dump(Table table, std::string filepath, bool with_type_info, bool quoted_strs) {
    std::ofstream(filepath) << csv_dumps(table, with_type_info, quoted_strs);
}

Table csv_load(std::string filepath, std::string table_name, bool with_type_info, bool quoted_strs) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return csv_loads(buffer.str(), table_name, with_type_info, quoted_strs);
}
