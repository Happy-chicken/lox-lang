#include "./include/vm.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include <llvm/IR/Constant.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <regex>
void LoxVM::exec(vector<shared_ptr<Stmt>> &statements) {


    // compile ast
    // compile(ast)
    compile(statements);

    module->print(llvm::outs(), nullptr);
    // 3. save module to file
    saveModuleToFile("./output.ll");
}

void LoxVM::saveModuleToFile(const std::string &fileName) {
    std::error_code errorCode;
    llvm::raw_fd_ostream outLL(fileName, errorCode);
    module->print(outLL, nullptr);
}

void LoxVM::compile(vector<shared_ptr<Stmt>> &statements) {
    fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false));

    gen(statements);

    // return 0
    builder->CreateRet(builder->getInt32(0));
}

llvm::Value *LoxVM::gen(vector<shared_ptr<Stmt>> &statements) {
    llvm::Value *ret = nullptr;
    // generate IR based on the AST (using visitor pattern)
    for (auto stmt: statements) {
        // codegenerate(stmt);
        switch (stmt->type) {
            case StmtType::Expression: {
                auto ExprStmt = std::dynamic_pointer_cast<Expression>(stmt);
                ret = evaluate(ExprStmt->expression);
                return builder->CreateRet(ret);
            }
            case StmtType::Print: {
                auto PrintStmt = std::dynamic_pointer_cast<Print>(stmt);
                auto expr = std::dynamic_pointer_cast<Call<Object>>(PrintStmt->expression);
                auto printFn = module->getFunction("printf");
                // auto str = builder->CreateGlobalStringPtr("Hello, World!");
                std::vector<llvm::Value *> args;
                for (auto arg: expr->arguments) {
                    args.push_back(evaluate(arg));
                }

                ret = builder->CreateCall(printFn, args);
                return ret;
            }
            case StmtType::Var: {
                auto VarStmt = std::dynamic_pointer_cast<Var>(stmt);
                auto varName = VarStmt->name.lexeme;
                if (VarStmt->initializer != nullptr) {
                    auto init = evaluate(VarStmt->initializer);
                    return createGlobalVariable(varName, (llvm::Constant *) init);
                } else {
                    // global variable without initializer to be assigned 0
                    return createGlobalVariable(varName, builder->getInt32(0));
                }
            }
        }
    }
    return builder->getInt32(0);
}

void LoxVM::moduleInit() {
    ctx = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("lox", *ctx);
    builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
}

llvm::Function *LoxVM::createFunction(const std::string &fnName, llvm::FunctionType *fnType) {
    // function prototype might already exist in the module
    auto fn = module->getFunction(fnName);
    if (fn == nullptr) {
        fn = createFunctionProto(fnName, fnType);
    }
    // create basic block
    createFunctionBlock(fn);
    return fn;
}

llvm::Function *LoxVM::createFunctionProto(const std::string &fnName, llvm::FunctionType *fnType) {
    auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, module.get());
    verifyFunction(*fn);

    return fn;
}

llvm::GlobalVariable *LoxVM::createGlobalVariable(const std::string &name, llvm::Constant *init) {
    module->getOrInsertGlobal(name, init->getType());
    auto variable = module->getNamedGlobal(name);
    variable->setConstant(false);
    variable->setInitializer(init);
    return variable;
}

/* create a basic block */
llvm::BasicBlock *LoxVM::createBB(const std::string &name, llvm::Function *fn = nullptr) {
    return llvm::BasicBlock::Create(*ctx, name, fn);
}

/* create a function block */
void LoxVM::createFunctionBlock(llvm::Function *fn) {
    auto entry = createBB("entry", fn);
    builder->SetInsertPoint(entry);
}

/* set up external functions like print*/
void LoxVM::setupExternalFunctions() {
    // void print(string)
    auto bytePtrTy = builder->getInt8PtrTy()->getPointerTo();
    module->getOrInsertFunction("printf", llvm::FunctionType::get(builder->getInt32Ty(), bytePtrTy, true));// true means varargs
}


void LoxVM::codegenerate(shared_ptr<Stmt> stmt) {
    stmt->accept(shared_from_this());
}

llvm::Value *LoxVM::evaluate(shared_ptr<Expr<Object>> expr) {
    // ir->print(llvm::outs());
    return expr->accept(shared_from_this()).llvmValue;
}

Object LoxVM::visitLiteralExpr(shared_ptr<Literal<Object>> expr) {
    //llvm to generate IR for literal
    llvm::Value *val = nullptr;
    switch (expr->value.data.index()) {
        case 0: {
            // string type
            std::string str_data = std::get<std::string>(expr->value.data);
            // handle the \n
            auto re = std::regex("\\\\n");
            str_data = std::regex_replace(str_data, re, "\n");
            val = builder->CreateGlobalStringPtr(str_data);
            return Object::make_llvmval_obj(val);
        }
        case 1: {
            // double type
            double dnumber_data = std::get<double>(expr->value.data);
            val = llvm::ConstantFP::get(builder->getDoubleTy(), dnumber_data);
            return Object::make_llvmval_obj(val);
        }
        case 2: {
            // bool type
            bool b_data = std::get<bool>(expr->value.data);
            val = builder->getInt1(b_data);
            return Object::make_llvmval_obj(val);
        }
    }


    return Object::make_nil_obj();
}

Object LoxVM::visitAssignExpr(shared_ptr<Assign<Object>> expr) {
    //llvm to generate IR for assignment
    return Object::make_nil_obj();
}

Object LoxVM::visitBinaryExpr(shared_ptr<Binary<Object>> expr) {
    //llvm to generate IR for binary
}

Object LoxVM::visitGroupingExpr(shared_ptr<Grouping<Object>> expr) {
    return Object::make_nil_obj();
}

Object LoxVM::visitUnaryExpr(shared_ptr<Unary<Object>> expr) {
    return Object::make_nil_obj();
}

Object LoxVM::visitVariableExpr(shared_ptr<Variable<Object>> expr) {
    // use the declared variable
    std::cout << "VariableExpr" << std::endl;
    return Object::make_nil_obj();
}

Object LoxVM::visitLogicalExpr(shared_ptr<Logical<Object>> expr) { return Object::make_nil_obj(); }

Object LoxVM::visitIncrementExpr(shared_ptr<Increment<Object>> expr) { return Object::make_nil_obj(); }

Object LoxVM::visitDecrementExpr(shared_ptr<Decrement<Object>> expr) { return Object::make_nil_obj(); }

Object LoxVM::visitListExpr(shared_ptr<List<Object>> expr) { return Object::make_nil_obj(); }

Object LoxVM::visitSubscriptExpr(shared_ptr<Subscript<Object>> expr) { return Object::make_nil_obj(); }

Object LoxVM::visitCallExpr(shared_ptr<Call<Object>> expr) {
    return Object::make_nil_obj();
}

Object LoxVM::visitGetExpr(shared_ptr<Get<Object>> expr) { return Object::make_nil_obj(); }

Object LoxVM::visitSetExpr(shared_ptr<Set<Object>> expr) { return Object::make_nil_obj(); }

Object LoxVM::visitThisExpr(shared_ptr<This<Object>> expr) { return Object::make_nil_obj(); }

Object LoxVM::visitSuperExpr(shared_ptr<Super<Object>> expr) { return Object::make_nil_obj(); }

void LoxVM::visitExpressionStmt(const Expression &stmt) {
    evaluate(stmt.expression);
}
void LoxVM::visitPrintStmt(const Print &stmt) {
    // evaluate(stmt.expression);
}
void LoxVM::visitVarStmt(const Var &stmt) {
    llvm::Value *init = nullptr;

    auto varName = stmt.name.lexeme;
    if (stmt.initializer != nullptr) {
        init = evaluate(stmt.initializer);
    }
}
void LoxVM::visitBlockStmt(const Block &stmt) {}
void LoxVM::visitClassStmt(const Class &stmt) {}
void LoxVM::visitIfStmt(const If &stmt) {
}
void LoxVM::visitWhileStmt(const While &stmt) {}
void LoxVM::visitFunctionStmt(shared_ptr<Function> stmt) {}
void LoxVM::visitReturnStmt(const Return &stmt) {}
void LoxVM::visitBreakStmt(const Break &stmt) {}
void LoxVM::visitContinueStmt(const Continue &stmt) {}
