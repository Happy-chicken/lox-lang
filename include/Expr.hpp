#ifndef EXPR_HPP_
#define EXPR_HPP_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Token.hpp"

using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;
using std::vector;

template<class R>
class Literal;

template<class R>
class Assign;

template<class R>
class Binary;

template<class R>
class Grouping;

template<class R>
class Unary;

template<class R>
class Variable;

template<class R>
class Logical;

template<class R>
class Increment;

template<class R>
class Decrement;

template<class R>
class List;

template<class R>
class Subscript;

template<class R>
class Call;

template<class R>
class Get;

template<class R>
class Set;

template<class R>
class This;

template<class R>
class Super;

template<class R>
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual R visitLiteralExpr(shared_ptr<Literal<R>> expr) = 0;
    virtual R visitAssignExpr(shared_ptr<Assign<R>> expr) = 0;
    virtual R visitBinaryExpr(shared_ptr<Binary<R>> expr) = 0;
    virtual R visitGroupingExpr(shared_ptr<Grouping<R>> expr) = 0;
    virtual R visitUnaryExpr(shared_ptr<Unary<R>> expr) = 0;
    virtual R visitIncrementExpr(shared_ptr<Increment<R>> expr) = 0;
    virtual R visitDecrementExpr(shared_ptr<Decrement<R>> expr) = 0;
    virtual R visitListExpr(shared_ptr<List<R>> expr) = 0;
    virtual R visitSubscriptExpr(shared_ptr<Subscript<R>> expr) = 0;
    // TODO
    // virtual R visitLambdaExpr(shared_ptr<Lambda<R>> expr) = 0;

    virtual R visitVariableExpr(shared_ptr<Variable<R>> expr) = 0;
    virtual R visitLogicalExpr(shared_ptr<Logical<R>> expr) = 0;
    virtual R visitCallExpr(shared_ptr<Call<R>> expr) = 0;
    virtual R visitGetExpr(shared_ptr<Get<R>> expr) = 0;
    virtual R visitSetExpr(shared_ptr<Set<R>> expr) = 0;
    virtual R visitThisExpr(shared_ptr<This<R>> expr) = 0;
    virtual R visitSuperExpr(shared_ptr<Super<R>> expr) = 0;
};

template<class R>
class Expr {
public:
    virtual R accept(shared_ptr<Visitor<R>> visitor) = 0;
    virtual ~Expr() = default;// for derived class
};

template<class R>
class Literal : public Expr<R>,
                public std::enable_shared_from_this<Literal<R>> {
public:
    explicit Literal(R value_) : value(value_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitLiteralExpr(this->shared_from_this());
    }
    R value;
};

template<class R>
class Assign : public Expr<R>, public std::enable_shared_from_this<Assign<R>> {
public:
    Assign(Token name_, shared_ptr<Expr<R>> value_)
        : name(name_), value(value_) {}

    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitAssignExpr(this->shared_from_this());
    }
    Token name;
    shared_ptr<Expr<R>> value;
};

template<class R>
class Binary : public Expr<R>, public std::enable_shared_from_this<Binary<R>> {
public:
    Binary(shared_ptr<Expr<R>> left_, Token operation, shared_ptr<Expr<R>> right_)
        : left(left_), operation(operation), right(right_) {}

    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitBinaryExpr(this->shared_from_this());
    }
    shared_ptr<Expr<R>> left;
    Token operation;
    shared_ptr<Expr<R>> right;
};

template<class R>
class Grouping : public Expr<R>,
                 public std::enable_shared_from_this<Grouping<R>> {
public:
    explicit Grouping(shared_ptr<Expr<R>> expression_)
        : expression(expression_) {}

    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitGroupingExpr(this->shared_from_this());
    }
    shared_ptr<Expr<R>> expression;
};

template<class R>
class Unary : public Expr<R>, public std::enable_shared_from_this<Unary<R>> {
public:
    Unary(Token operation, shared_ptr<Expr<R>> right_)
        : operation(operation), right(right_) {}

    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitUnaryExpr(this->shared_from_this());
    }
    Token operation;
    shared_ptr<Expr<R>> right;
};

template<class R>
class Increment : public Expr<R>,
                  public std::enable_shared_from_this<Increment<R>> {
public:
    typedef enum { POSTFIX,
                   PREFIX } Type;
    Increment(Token identifier_, Type type_)
        : identifier(identifier_), type(type_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitIncrementExpr(this->shared_from_this());
    }

    Token identifier;
    Type type;
};

template<class R>
class Decrement : public Expr<R>,
                  public std::enable_shared_from_this<Decrement<R>> {
public:
    typedef enum { POSTFIX,
                   PREFIX } Type;
    Decrement(Token identifier_, Type type_)
        : identifier(identifier_), type(type_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitDecrementExpr(this->shared_from_this());
    }
    Token identifier;
    Type type;
};

template<class R>
class Variable : public Expr<R>,
                 public std::enable_shared_from_this<Variable<R>> {
public:
    explicit Variable(Token name_) : name(name_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitVariableExpr(this->shared_from_this());
    };
    Token name;
};

template<class R>
class Logical : public Expr<R>,
                public std::enable_shared_from_this<Logical<R>> {
public:
    Logical(shared_ptr<Expr<R>> left_, Token operation_, shared_ptr<Expr<R>> right_)
        : left(left_), operation(operation_), right(right_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitLogicalExpr(this->shared_from_this());
    }
    shared_ptr<Expr<R>> left;
    Token operation;
    shared_ptr<Expr<R>> right;
};

template<class R>
class List : public Expr<R>, public std::enable_shared_from_this<List<R>> {
public:
    Token opening_bracket;
    vector<shared_ptr<Expr<R>>> items;

    List(Token opening_bracket_, vector<shared_ptr<Expr<R>>> items_)
        : opening_bracket(opening_bracket_), items(items_) {}

    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitListExpr(this->shared_from_this());
    }
};

template<class R>
class Subscript : public Expr<R>,
                  public std::enable_shared_from_this<Subscript<R>> {
public:
    Token identifier;
    shared_ptr<Expr<R>> index;
    shared_ptr<Expr<R>> value;

    Subscript(Token identifier_, shared_ptr<Expr<R>> index_, shared_ptr<Expr<R>> value_)
        : identifier(identifier_), index(index_), value(value_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitSubscriptExpr(this->shared_from_this());
    }
};

template<class R>
class Call : public Expr<R>, public std::enable_shared_from_this<Call<R>> {
public:
    Call(shared_ptr<Expr<R>> callee_, Token paren_, vector<shared_ptr<Expr<R>>> arguments_)
        : callee(callee_), paren(paren_), arguments(arguments_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitCallExpr(this->shared_from_this());
    }
    shared_ptr<Expr<R>> callee;
    Token paren;
    vector<shared_ptr<Expr<R>>> arguments;
};

template<class R>
class Get : public Expr<R>, public std::enable_shared_from_this<Get<R>> {
public:
    Get(shared_ptr<Expr<R>> object_, Token name_)
        : object(object_), name(name_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitGetExpr(this->shared_from_this());
    }
    shared_ptr<Expr<R>> object;
    Token name;
};

template<class R>
class Set : public Expr<R>, public std::enable_shared_from_this<Set<R>> {
public:
    Set(shared_ptr<Expr<R>> object_, Token name_, shared_ptr<Expr<R>> value_)
        : object(object_), name(name_), value(value_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitSetExpr(this->shared_from_this());
    }
    shared_ptr<Expr<R>> object;
    Token name;
    shared_ptr<Expr<R>> value;
};

template<class R>
class This : public Expr<R>, public std::enable_shared_from_this<This<R>> {
public:
    explicit This(Token keyword_) : keyword(keyword_) {}
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitThisExpr(this->shared_from_this());
    }
    Token keyword;
};

template<class R>
class Super : public Expr<R>, public std::enable_shared_from_this<Super<R>> {
public:
    Super(Token keyword_, Token method_) : keyword(keyword_), method(method_){};
    R accept(shared_ptr<Visitor<R>> visitor) override {
        return visitor->visitSuperExpr(this->shared_from_this());
    };
    Token keyword;
    Token method;
};
// TODO
// template <class R>
// class Lambda : Expr<R>
// {
//     /*a lambda exporession */
//     vector<Token> params;
//     shared_ptr<Stmt> body;

//     Lambda(vector<Token> params_, shared_ptr<Stmt> body_)
//         : params(params_), body(body_)
//     {
//     }

//     R accept(shared_ptr<Visitor<R>> visitor) override
//     {
//         return visitor->visitLambdaExpr(this->shared_from_this());
//     }
// };
#endif// EXPR_HPP_
