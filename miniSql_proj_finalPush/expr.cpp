// expr.cpp
#include "expr.hpp"
#include "row.hpp"
bool Expr::truthy(Row row) { return eval(row).truthy(); }
BinaryOp::BinaryOp(ExprPtr l, ExprPtr r) : left(l), right(r) {}
UnaryOp::UnaryOp(ExprPtr op) : operand(op) {}

class Op_Add : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        CellData l = left->eval(row);
        CellData r = right->eval(row);
        return l.type == DataType::INTEGER && r.type == DataType::INTEGER ?
            CellData(int(l) + int(r)) : CellData(double(l) + double(r));
    }
};

class Op_Subtract : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        CellData l = left->eval(row);
        CellData r = right->eval(row);
        return l.type == DataType::INTEGER && r.type == DataType::INTEGER ?
            CellData(int(l) - int(r)) : CellData(double(l) - double(r));
    }
};

class Op_Multiply : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        CellData l = left->eval(row);
        CellData r = right->eval(row);
        return l.type == DataType::INTEGER && r.type == DataType::INTEGER ?
            CellData(int(l) * int(r)) : CellData(double(l) * double(r));
    }
};

class Op_Divide : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        CellData l = left->eval(row);
        CellData r = right->eval(row);
        double divisor = double(r);
        if(divisor == 0.0) throw std::runtime_error("Division by zero");
        if(l.type == DataType::INTEGER && r.type == DataType::INTEGER) {
            int i_divisor = int(r);
            if(i_divisor != 0 && int(l) % i_divisor == 0) {
                return CellData(int(l) / i_divisor);
            }
        }
        return CellData(double(l) / divisor);
    }
};

class Op_Less : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        CellData l = left->eval(row);
        CellData r = right->eval(row);
        if(l.type == DataType::TEXT || r.type == DataType::TEXT) {
            return CellData(std::string(l) < std::string(r));
        }
        if(l.type == DataType::INTEGER && r.type == DataType::INTEGER) {
            return CellData(int(l) < int(r));
        }
        return CellData(double(l) < double(r));
    }
};

class Op_Equal : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        CellData l = left->eval(row);
        CellData r = right->eval(row);
        if(l.type == DataType::TEXT || r.type == DataType::TEXT) {
            return CellData(std::string(l) == std::string(r));
        }
        if(l.type == DataType::INTEGER && r.type == DataType::INTEGER) {
            return CellData(int(l) == int(r));
        }
        return CellData(double(l) == double(r));
    }
};

class Op_Greater : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        CellData l = left->eval(row);
        CellData r = right->eval(row);
        if(l.type == DataType::TEXT || r.type == DataType::TEXT) {
            return CellData(std::string(l) > std::string(r));
        }
        if(l.type == DataType::INTEGER && r.type == DataType::INTEGER) {
            return CellData(int(l) > int(r));
        }
        return CellData(double(l) > double(r));
    }
};

class Op_And : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        return CellData(left->truthy(row) && right->truthy(row));
    }
};

class Op_Or : public BinaryOp {
public:
    using BinaryOp::BinaryOp;
    CellData eval(Row row) override {
        return CellData(left->truthy(row) || right->truthy(row));
    }
};

class Op_Not : public UnaryOp {
public:
    using UnaryOp::UnaryOp;
    CellData eval(Row row) override {
        return CellData(!operand->truthy(row));
    }
};

CellData ColRef::eval(Row row) {
    return row[name];
}

ExprPtr operator+(ExprPtr l, ExprPtr r) { return std::make_shared<Op_Add>(l, r); }
ExprPtr operator-(ExprPtr l, ExprPtr r) { return std::make_shared<Op_Subtract>(l, r); }
ExprPtr operator*(ExprPtr l, ExprPtr r) { return std::make_shared<Op_Multiply>(l, r); }
ExprPtr operator/(ExprPtr l, ExprPtr r) { return std::make_shared<Op_Divide>(l, r); }
ExprPtr operator<(ExprPtr l, ExprPtr r) { return std::make_shared<Op_Less>(l, r); }
ExprPtr operator==(ExprPtr l, ExprPtr r) { return std::make_shared<Op_Equal>(l, r); }
ExprPtr operator>(ExprPtr l, ExprPtr r) { return std::make_shared<Op_Greater>(l, r);}
    // Add these implementations
    ExprPtr operator+(ExprPtr l, CellData r) { return l + literal(r); }
    ExprPtr operator+(CellData l, ExprPtr r) { return literal(l) + r; }
    ExprPtr operator-(ExprPtr l, CellData r) { return l - literal(r); }
    ExprPtr operator-(CellData l, ExprPtr r) { return literal(l) - r; }
    ExprPtr operator*(ExprPtr l, CellData r) { return l * literal(r); }
    ExprPtr operator*(CellData l, ExprPtr r) { return literal(l) * r; }
    ExprPtr operator/(ExprPtr l, CellData r) { return l / literal(r); }
    ExprPtr operator/(CellData l, ExprPtr r) { return literal(l) / r; }
    ExprPtr operator<(ExprPtr l, CellData r) { return l < literal(r); }
    ExprPtr operator<(CellData l, ExprPtr r) { return literal(l) < r; }
    ExprPtr operator==(ExprPtr l, CellData r) { return l == literal(r); }
    ExprPtr operator==(CellData l, ExprPtr r) { return literal(l) == r; }
    ExprPtr operator>(ExprPtr l, CellData r) { return l > literal(r); }
    ExprPtr operator>(CellData l, ExprPtr r) { return literal(l) > r; }
ExprPtr operator&&(ExprPtr l, ExprPtr r) { return std::make_shared<Op_And>(l, r); }
ExprPtr operator||(ExprPtr l, ExprPtr r) { return std::make_shared<Op_Or>(l, r); }
ExprPtr operator!(ExprPtr r) { return std::make_shared<Op_Not>(r); }


ColRef::ColRef(std::string name) : name(name){}
Literal::Literal(CellData data): value(data){}

// expr.cpp - add implementations:
ExprPtr col(std::string name) {
    return std::make_shared<ColRef>(std::move(name));
}

ExprPtr literal(CellData value) {
    return std::make_shared<Literal>(std::move(value));
}

CellData Literal::eval(Row row){
    return value;
}
