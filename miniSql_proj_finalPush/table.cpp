#include "table.hpp"

#include "table.hpp"

Table::Table(std::string name, Schema schema) : name(name), schema(schema) {}

Table::Table(const Table& other) : name(other.name), schema(other.schema), rows(other.rows) {}

Table::Table(Table&& other) noexcept
    : name(std::move(other.name)), schema(std::move(other.schema)), rows(std::move(other.rows)) {}

Table& Table::operator=(const Table& other) {
    name = other.name;
    schema = other.schema;
    rows = other.rows;
    return *this;
}

Table& Table::operator=(Table&& other) noexcept {
    name = std::move(other.name);
    schema = std::move(other.schema);
    rows = std::move(other.rows);
    return *this;
}

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

// table.cpp
void Table::update_where(ExprPtr condition, NamedVector<ExprPtr> values) {
    for (auto& row : rows) {
        if (condition->truthy(row)) {
            for (auto& value : values.elements) {
                row.cells[value.name] = value.value->eval(row);
            }
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

// table.cpp
Table Table::join(Table& other) {
    Schema joined_schema;
    
    for (const auto& elem : schema.elements) {
        joined_schema[name + "." + elem.name] = elem.value;
    }
    for (const auto& elem : other.schema.elements) {
        joined_schema[other.name + "." + elem.name] = elem.value;
    }
    
    Table joined(name + "_x_" + other.name, joined_schema);
    
    for (auto& row1 : rows) {
        for (auto& row2 : other.rows) {
            Row joined_row(joined_schema);
            for (const auto& elem : schema.elements) {
                joined_row.cells[name + "." + elem.name] = row1.cells[elem.name];
            }
            for (const auto& elem : other.schema.elements) {
                joined_row.cells[other.name + "." + elem.name] = row2.cells[elem.name];
            }
            joined.rows.push_back(joined_row);
        }
    }
    
    return joined;
}
