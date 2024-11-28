#ifndef NAMED_VECTOR_H
#define NAMED_VECTOR_H

#include <vector>
#include <string>
#include <stdexcept>

template<typename T>
class NamedVector {
public:
    struct Element {
        std::string name;
        T value;
        Element(std::string name, T value) : name(name), value(value) {}
    };
    
    std::vector<Element> elements;
    
    T& operator[](size_t index) {
        if(index >= elements.size()) throw std::out_of_range("Index out of range");
        return elements[index].value;
    }
    
    T& operator[](std::string name) {
        for(auto& elem : elements) {
            if(elem.name == name) return elem.value;
        }
        elements.push_back(Element(name, T()));
        return elements.back().value;
    }
    
    size_t size() const { return elements.size(); }
};
#endif
