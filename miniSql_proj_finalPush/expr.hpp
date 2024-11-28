// expr.hpp
#ifndef EXPR_H
#define EXPR_H

#include "celldata.hpp"
#include <memory>

class Row;  // Forward declaration

class Expr {
public:
   virtual CellData eval(Row row) = 0;
   virtual bool truthy(Row row);
   virtual ~Expr() = default;
};

using ExprPtr = std::shared_ptr<Expr>;

class BinaryOp : public Expr {
public:
   ExprPtr left;
   ExprPtr right;
   BinaryOp(ExprPtr l, ExprPtr r);
};

class UnaryOp : public Expr {
public:
   ExprPtr operand;
   UnaryOp(ExprPtr op);
};

// Operation classes declarations

class ColRef : public Expr {
public:
   std::string name;
   ColRef(std::string n);
   CellData eval(Row row) override;
    
    virtual ~ColRef() = default;
};

class Literal : public Expr {
public:
   CellData value;
   Literal(CellData v);
   CellData eval(Row row) override;
    
     virtual ~Literal() = default;
};

ExprPtr col(std::string name);
ExprPtr literal(CellData value);

ExprPtr operator+(ExprPtr l, ExprPtr r);
ExprPtr operator-(ExprPtr l, ExprPtr r);
ExprPtr operator*(ExprPtr l, ExprPtr r);
ExprPtr operator/(ExprPtr l, ExprPtr r);
ExprPtr operator<(ExprPtr l, ExprPtr r);
ExprPtr operator==(ExprPtr l, ExprPtr r);
ExprPtr operator>(ExprPtr l, ExprPtr r);
ExprPtr operator&&(ExprPtr l, ExprPtr r);
ExprPtr operator||(ExprPtr l, ExprPtr r);
ExprPtr operator!(ExprPtr r);

// Add these declarations
ExprPtr operator+(ExprPtr l, CellData r);
ExprPtr operator+(CellData l, ExprPtr r);
ExprPtr operator-(ExprPtr l, CellData r);
ExprPtr operator-(CellData l, ExprPtr r);
ExprPtr operator*(ExprPtr l, CellData r);
ExprPtr operator*(CellData l, ExprPtr r);
ExprPtr operator/(ExprPtr l, CellData r);
ExprPtr operator/(CellData l, ExprPtr r);
ExprPtr operator<(ExprPtr l, CellData r);
ExprPtr operator<(CellData l, ExprPtr r);
ExprPtr operator==(ExprPtr l, CellData r);
ExprPtr operator==(CellData l, ExprPtr r);
ExprPtr operator>(ExprPtr l, CellData r);
ExprPtr operator>(CellData l, ExprPtr r);

#endif
