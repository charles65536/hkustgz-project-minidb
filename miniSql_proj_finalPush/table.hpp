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
    bool isJoinedTable;
    
    Table(std::string name, Schema schema, bool isJoined = false)
            : name(std::move(name)), schema(std::move(schema)), isJoinedTable(isJoined) {}
        
    Table() = default;
    
    // Copy/Move semantics
    Table(const Table& other);
    Table(Table&& other) noexcept;
    Table& operator=(const Table& other);
    Table& operator=(Table&& other) noexcept;
    
    void append_row(Row row);
    Table where(ExprPtr condition);
    void delete_where(ExprPtr condition);
    void update_where(ExprPtr condition, std::string col_name, ExprPtr new_value);
    // table.hpp
    void update_where(ExprPtr condition, NamedVector<ExprPtr> values);
    Table select(std::vector<std::string> cols);
    // table.hpp
    Table join(Table& other);
};
#endif


