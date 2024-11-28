#include "row.hpp"

Row::Row(Schema schema) : schema(schema) {
    for(size_t i = 0; i < schema.elements.size(); i++) {
        cells[schema.elements[i].name] = CellData(schema[i]);
    }
}

CellData Row::operator[](std::string name) {
    return cells[name];
}

CellData Row::operator[](size_t index) {
    return cells[index];
}
