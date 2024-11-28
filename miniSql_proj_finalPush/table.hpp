// table.hpp
#ifndef TABLE_H
#define TABLE_H

#include "row.hpp"
#include "expr.hpp"
#include <list>

class Table {
public:
    std::string name;
    Schema schema;
    std::list<Row> rows;
    
    Table() = default;
    Table(std::string name, Schema schema);
    
    void append_row(Row row);
    Table where(ExprPtr condition);
    void delete_where(ExprPtr condition);
    void update_where(ExprPtr condition, std::string col_name, ExprPtr new_value);
    Table select(std::vector<std::string> cols);
};

#endif
