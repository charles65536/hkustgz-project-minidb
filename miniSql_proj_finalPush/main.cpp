// main.cpp
#include "table.hpp"
#include "csv_manip.hpp"
#include <iostream>

int main() {
    Schema schema;
    schema["id"] = DataType::INTEGER;
    schema["name"] = DataType::TEXT;
    schema["score"] = DataType::INTEGER;
    
    Table students("students", schema);
    
    Row r1(schema);
    r1.cells["id"] = CellData(1);
    r1.cells["name"] = CellData("Alice");
    r1.cells["score"] = CellData(95);
    students.append_row(r1);
    
    Row r2(schema);
    r2.cells["id"] = CellData(2);
    r2.cells["name"] = CellData("Bob");
    r2.cells["score"] = CellData(87);
    students.append_row(r2);
    
    // Test operations
    auto high_scores = students.where(col("score") > 90);
    std::cout << "High scores:\n" << csv_dumps(high_scores);
    
    students.update_where(col("score") < literal(CellData(90)),
                         "score",
                         col("score") + literal(CellData(5)));
                         
    students.delete_where(col("id") == literal(CellData(1)));
    
    std::cout << "\nFinal table:\n" << csv_dumps(students);
    return 0;
}
