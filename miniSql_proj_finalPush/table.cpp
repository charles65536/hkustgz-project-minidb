#include "table.hpp"

Table::Table(std::string name, Schema schema) : name(name), schema(schema) {}

void Table::append_row(Row row) {
    if(row.schema.elements.size() != schema.elements.size()) {
        throw std::runtime_error("Row schema size mismatch");
    }
    for(size_t i = 0; i < schema.elements.size(); i++) {
        if(schema.elements[i].value != row.schema.elements[i].value) {
            throw std::runtime_error("Schema type mismatch");
        }
    }
    rows.push_back(row);
}

Table Table::where(ExprPtr condition) {
    Table result(name + "_filtered", schema);
    for(auto& row : rows) {
        if(condition->truthy(row)) {
            result.rows.push_back(row);
        }
    }
    return result;
}

void Table::delete_where(ExprPtr condition) {
    rows.remove_if([&](Row& row) { return condition->truthy(row); });
}

void Table::update_where(ExprPtr condition, std::string col_name, ExprPtr new_value) {
    for(auto& row : rows) {
        if(condition->truthy(row)) {
            row.cells[col_name] = new_value->eval(row);
        }
    }
}

Table Table::select(std::vector<std::string> cols) {
    Schema new_schema;
    for(auto& col : cols) {
        new_schema[col] = schema[col];
    }
    
    Table result(name + "_projected", new_schema);
    for(auto& row : rows) {
        Row new_row(new_schema);
        for(auto& col : cols) {
            new_row.cells[col] = row.cells[col];
        }
        result.rows.push_back(new_row);
    }
    return result;
}
