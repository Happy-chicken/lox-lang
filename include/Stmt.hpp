#ifndef STMT_HPP_
#define STMT_HPP_

#include "Expr.hpp"
#include "Token.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using std::cout;
using std::endl;
using std::shared_ptr;
using std::string;
using std::vector;

class Expression;
class Print;
class Var;
class Block;
class If;
class While;
class Function;
class Return;
class Break;
class Continue;
class Class;

class Visitor_Stmt {
public:
    virtual ~Visitor_Stmt() = default;
    virtual void visitExpressionStmt(const Expression &stmt) = 0;
    virtual void visitPrintStmt(const Print &stmt) = 0;
    virtual void visitVarStmt(const Var &stmt) = 0;
    virtual void visitBlockStmt(const Block &stmt) = 0;
    virtual void visitIfStmt(const If &stmt) = 0;
    virtual void visitWhileStmt(const While &stmt) = 0;
    virtual void visitFunctionStmt(shared_ptr<Function> stmt) = 0;
    virtual void visitReturnStmt(const Return &stmt) = 0;
    virtual void visitBreakStmt(const Break &stmt) = 0;
    virtual void visitContinueStmt(const Continue &stmt) = 0;
    virtual void visitClassStmt(const Class &stmt) = 0;
};

enum class StmtType {
    Expression,
    Print,
    Var,
    Block,
    If,
    While,
    Function,
    Return,
    Break,
    Continue,
    Class
};

class Stmt {
public:
    virtual void accept(shared_ptr<Visitor_Stmt> visitor) = 0;
    virtual ~Stmt() = default;// for derived class
    StmtType type;
};

class Expression : public Stmt {
public:
    explicit Expression(shared_ptr<Expr<Object>> expression_)
        : expression(expression_) { type = StmtType::Expression; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitExpressionStmt(*this);
    }
    shared_ptr<Expr<Object>> expression;
};

class Var : public Stmt {
public:
    explicit Var(Token name_, shared_ptr<Expr<Object>> initializer_, string typeName_)
        : initializer(initializer_), name(name_), typeName(typeName_) { type = StmtType::Var; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitVarStmt(*this);
    }
    shared_ptr<Expr<Object>> initializer;
    Token name;
    string typeName;
};

class Block : public Stmt {
public:
    explicit Block(vector<shared_ptr<Stmt>> statements_)
        : statements(statements_) { type = StmtType::Block; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitBlockStmt(*this);
    }
    vector<shared_ptr<Stmt>> statements;
};

struct IfBranch {
    shared_ptr<Expr<Object>> condition;
    shared_ptr<Stmt> statement;

    IfBranch(shared_ptr<Expr<Object>> condition, shared_ptr<Stmt> statement)
        : condition{std::move(condition)}, statement{std::move(statement)} {
        assert(this->condition != nullptr);
        assert(this->statement != nullptr);
    }
};

class If : public Stmt {
public:
    If(IfBranch main_branch_, vector<IfBranch> elif_branches_,
       shared_ptr<Stmt> else_branch_)
        : main_branch(main_branch_), elif_branches(elif_branches_),
          else_branch(else_branch_) { type = StmtType::If; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitIfStmt(*this);
    }
    IfBranch main_branch;
    vector<IfBranch> elif_branches;
    shared_ptr<Stmt> else_branch;
};

class While : public Stmt {
public:
    While(shared_ptr<Expr<Object>> condition_, shared_ptr<Stmt> body_)
        : condition(condition_), body(body_) { type = StmtType::While; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitWhileStmt(*this);
    }
    shared_ptr<Expr<Object>> condition;
    shared_ptr<Stmt> body;
};

class Function : public Stmt, public std::enable_shared_from_this<Function> {
public:
    Function(Token name_, vector<std::pair<Token, string>> params_, vector<shared_ptr<Stmt>> body_, Token returnTypeName_)
        : functionName(name_), params(params_), body(body_), returnTypeName(returnTypeName_) { type = StmtType::Function; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitFunctionStmt(this->shared_from_this());
    }
    Token functionName;
    vector<std::pair<Token, string>> params;
    vector<shared_ptr<Stmt>> body;
    Token returnTypeName;
};

class Print : public Stmt {
public:
    explicit Print(shared_ptr<Expr<Object>> expression_)
        : expression(expression_) { type = StmtType::Print; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitPrintStmt(*this);
    }
    shared_ptr<Expr<Object>> expression;
};

class Return : public Stmt {
public:
    Return(Token name_, shared_ptr<Expr<Object>> value_)
        : name(name_), value(value_) { type = StmtType::Return; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitReturnStmt(*this);
    }
    Token name;
    shared_ptr<Expr<Object>> value;
};

class Continue : public Stmt {
public:
    Continue(Token keyword_) : keyword(keyword_) { type = StmtType::Continue; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitContinueStmt(*this);
    }
    Token keyword;
};

class Break : public Stmt {
public:
    Break(Token keyword_) : keyword(keyword_) { type = StmtType::Break; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitBreakStmt(*this);
    }
    Token keyword;
};

class Class : public Stmt {
public:
    Class(Token name_, shared_ptr<Variable<Object>> superclass_, vector<shared_ptr<Function>> methods_, shared_ptr<Block> body_)
        : name(name_), superclass(superclass_), methods(methods_), body(body_) { type = StmtType::Class; }
    void accept(shared_ptr<Visitor_Stmt> visitor) override {
        visitor->visitClassStmt(*this);
    }
    Token name;
    shared_ptr<Variable<Object>> superclass;
    vector<shared_ptr<Function>> methods;
    shared_ptr<Block> body;
};

#endif// STMT_HPP_
