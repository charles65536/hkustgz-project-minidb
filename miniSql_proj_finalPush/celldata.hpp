#ifndef CELLDATA_H
#define CELLDATA_H

#include <string>
#include <ostream>

enum class DataType {
    INTEGER,
    FLOAT,
    TEXT
};

class CellData {
public:
    DataType type;
    CellData() = default;
    CellData(DataType type);
    CellData(DataType type, std::string initializer);
    
    CellData(int value) : type(DataType::INTEGER), data_integer(value) {}
    CellData(double value) : type(DataType::FLOAT), data_float(value) {}
    CellData(const char* value) : type(DataType::TEXT), data_text(value) {}
    CellData(std::string value) : type(DataType::TEXT), data_text(std::move(value)) {}

    void read(std::string initializer);
    
    operator std::string() const;
    operator int() const;
    operator double() const;
    
    friend std::ostream& operator<<(std::ostream& os, const CellData& cell);
    bool truthy() const;
    
    
    std::partial_ordering operator<=>(const CellData& other) const {
            if (type == DataType::TEXT || other.type == DataType::TEXT) {
                return std::string(*this) <=> std::string(other);
            }
            if (type == DataType::INTEGER && other.type == DataType::INTEGER) {
                return int(*this) <=> int(other);
            }
            return double(*this) <=> double(other);
        }
        
        bool operator==(const CellData& other) const = default;

private:
    int data_integer;
    double data_float;
    std::string data_text;
};

DataType infer_datatype(const std::string& literal);

CellData inferred_cell(const std::string& literal) ;

#endif
