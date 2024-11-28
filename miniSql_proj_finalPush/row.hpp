#ifndef ROW_H
#define ROW_H

#include "schema.hpp"
#include "named_vector.hpp"

class Row {
public:
    Schema schema;
    NamedVector<CellData> cells;
    
    Row(Schema schema);
    CellData operator[](std::string name);
    CellData operator[](size_t index);
};
#endif
