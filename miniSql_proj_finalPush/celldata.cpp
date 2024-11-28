#include "celldata.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>

CellData::CellData(DataType type) : type(type) {
    switch (type) {
        case DataType::INTEGER: data_integer = 0; break;
        case DataType::FLOAT: data_float = 0.0; break;
        case DataType::TEXT: data_text = ""; break;
    }
}

CellData::CellData(DataType type, std::string initializer) : type(type) {
    read(std::move(initializer));
}

void CellData::read(std::string initializer) {
    try {
        switch (type) {
            case DataType::INTEGER:
                data_integer = std::stoi(initializer);
                break;
            case DataType::FLOAT:
                data_float = std::stod(initializer);
                break;
            case DataType::TEXT:
                data_text = std::move(initializer);
                break;
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to convert '" + initializer +
                               "' to type " + std::to_string(static_cast<int>(type)));
    }
}

CellData::operator std::string() const {
   std::ostringstream os;
   os.precision(2);
   os << std::fixed;
   switch(type) {
       case DataType::INTEGER: return std::to_string(data_integer);
       case DataType::FLOAT: os << data_float; return os.str();
       case DataType::TEXT: return data_text;
   }
   return "";
}

CellData::operator int() const {
    switch (type) {
        case DataType::INTEGER:
            return data_integer;
        case DataType::FLOAT:
            return static_cast<int>(data_float);
        case DataType::TEXT:
            try {
                return std::stoi(data_text);
            } catch (const std::exception& e) {
                throw std::runtime_error("Cannot convert text '" + data_text + "' to integer");
            }
    }
    throw std::runtime_error("Unknown data type");
}

CellData::operator double() const {
    switch (type) {
        case DataType::INTEGER:
            return static_cast<double>(data_integer);
        case DataType::FLOAT:
            return data_float;
        case DataType::TEXT:
            try {
                return std::stod(data_text);
            } catch (const std::exception& e) {
                throw std::runtime_error("Cannot convert text '" + data_text + "' to float");
            }
    }
    throw std::runtime_error("Unknown data type");
}

bool CellData::truthy() const {
    switch (type) {
        case DataType::INTEGER: return data_integer != 0;
        case DataType::FLOAT: return data_float != 0.0;
        case DataType::TEXT: return !data_text.empty();
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const CellData& cell) {
    return os << static_cast<std::string>(cell);
}
