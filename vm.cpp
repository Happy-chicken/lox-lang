#include "./include/vm.hpp"
#include "Environment.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <regex>
#include <string>

llvm::Value *LoxVM::lastValue = nullptr;
void LoxVM::exec(vector<shared_ptr<Stmt>> &statements) {
    // 1. compile ast
    compile(statements);
    // 2. print llvm IR
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
    fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false), globalEnv);

    gen(statements);

    // return 0
    builder->CreateRet(builder->getInt32(0));
}

llvm::Value *LoxVM::gen(vector<shared_ptr<Stmt>> &statements) {
    // generate IR based on the AST (using visitor pattern)
    for (auto stmt: statements) {
        execute(stmt);
    }
    return LoxVM::lastValue;
}

void LoxVM::moduleInit() {
    ctx = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("lox", *ctx);
    builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    varsBuilder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    LoxVM::lastValue = builder->getInt32(0);
}

llvm::Function *LoxVM::createFunction(const std::string &fnName, llvm::FunctionType *fnType, Env env) {
    // function prototype might already exist in the module
    auto fn = module->getFunction(fnName);
    if (fn == nullptr) {
        fn = createFunctionProto(fnName, fnType, env);
    }
    // create basic block
    createFunctionBlock(fn);
    return fn;
}

llvm::Function *LoxVM::createFunctionProto(const std::string &fnName, llvm::FunctionType *fnType, Env env) {
    auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, module.get());
    verifyFunction(*fn);

    env->define(fnName, fn);
    return fn;
}

llvm::Value *LoxVM::allocVar(const std::string &name, llvm::Type *type, Env env) {
    varsBuilder->SetInsertPoint(&fn->getEntryBlock());
    auto varAlloc = varsBuilder->CreateAlloca(type, 0, name.c_str());

    env->define(name, varAlloc);
    return varAlloc;
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

void LoxVM::setupGlobalEnvironment() {
    std::map<string, llvm::Value *> globalObjects{
        {"LOX_VERSION", builder->getInt32(42)},
    };
    std::map<string, llvm::Value *> globalRecords{};

    for (auto &entry: globalObjects) {
        globalRecords[entry.first] = createGlobalVariable(entry.first, (llvm::Constant *) entry.second);
    }
    globalEnv = std::make_shared<Environment>(nullptr, globalRecords);
}

llvm::Type *LoxVM::excrateVarType(std::shared_ptr<Expr<Object>> expr) {
    if (std::dynamic_pointer_cast<Literal<Object>>(expr) != nullptr) {
        auto literal = std::dynamic_pointer_cast<Literal<Object>>(expr);
        switch (literal->value.data.index()) {
            case 0:
                // string type
                return builder->getInt8PtrTy()->getPointerTo();
            case 1:
                // double type
                return builder->getDoubleTy();
            case 2:
                // bool type
                return builder->getInt1Ty();
            case 8:
                // int type
                return builder->getInt32Ty();
        }
    }
    return builder->getInt32Ty();
}

llvm::Type *LoxVM::excrateVarType(const string &typeName) {

    if (typeName == "int") {
        return builder->getInt32Ty();
    } else if (typeName == "str") {
        return builder->getInt8PtrTy()->getPointerTo();
    } else if (typeName == "") {
        return builder->getInt32Ty();
    }
    return classMap_[typeName].cls->getPointerTo();
}

bool LoxVM::hasReturnType(shared_ptr<Stmt> stmt) {
    // if
    // return fnExp.list[3].type == ExpType::SYMBOL && fnExp.list[3].string == "->";
    return false;
}

// realted to class helper function
llvm::StructType *LoxVM::getClassByName(const std::string &name) {
    return llvm::StructType::getTypeByName(*ctx, name);
}

void LoxVM::inheritClass(llvm::StructType *cls, llvm::StructType *parent) {}

void LoxVM::buildClassInfo(llvm::StructType *cls, const Class &stmt, Env env) {
    auto className = stmt.name.lexeme;
    auto classInfo = &classMap_[className];

    auto methods = stmt.methods;

    // member variable need to be added outside of the class
    // add using code like cls.a=1; and visit as well

    // now suppport declare variable in class
    for (auto member: stmt.members) {
        auto fieldName = member->name.lexeme;
        auto fieldType = excrateVarType(member->typeName);

        classInfo->fieldsMap[fieldName] = fieldType;
    }
    for (auto method: methods) {
        // method
        auto methodName = method->functionName.lexeme;
        auto fnName = className + "_" + methodName;
        classInfo->methodsMap[methodName] = createFunctionProto(fnName, excrateFunType(method), env);
    }

    // create field
    buildClassBody(cls);
}

// build class body
void LoxVM::buildClassBody(llvm::StructType *cls) {
    std::string className{cls->getName().data()};

    auto classInfo = &classMap_[className];
    auto clsFields = std::vector<llvm::Type *>{};
    for (const auto &field: classInfo->fieldsMap) {
        clsFields.push_back(field.second);
    }

    cls->setBody(clsFields, false);

    // methods
}

llvm::FunctionType *LoxVM::excrateFunType(shared_ptr<Function> stmt) {
    auto params = stmt->params;
    auto returnType = stmt->returnTypeName.type == NIL ? builder->getInt32Ty()
                                                       : excrateVarType(stmt->returnTypeName.lexeme);
    // auto returnType = builder->getInt32Ty();
    std::vector<llvm::Type *> paramTypes{};
    for (auto &param: params) {
        auto paramType = excrateVarType(param.second);
        // auto paramType = builder->getInt32Ty();
        auto paramName = param.first.lexeme;
        paramTypes.push_back(paramName == "self" ? (llvm::Type *) cls->getPointerTo() : paramType);
    }
    return llvm::FunctionType::get(returnType, paramTypes, false);
    // return llvm::FunctionType::get(returnType, paramTypes, false);
}


void LoxVM::executeBlock(vector<shared_ptr<Stmt>> statements, Env env) {
    EnvironmentGuard env_guard{*this, env};
    for (auto stmt: statements) {
        execute(stmt);
    }
}

void LoxVM::execute(shared_ptr<Stmt> stmt) {
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
        case 8: {
            // bool type
            int i32_data = std::get<int>(expr->value.data);
            val = builder->getInt32(i32_data);
            return Object::make_llvmval_obj(val);
        }
    }


    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitAssignExpr(shared_ptr<Assign<Object>> expr) {
    //llvm to generate IR for assignment
    auto varName = expr->name.lexeme;
    auto value = evaluate(expr->value);
    // variable
    auto varBinding = this->environment->lookup(varName);

    builder->CreateStore(value, varBinding);
    return Object::make_llvmval_obj(value);
}

Object LoxVM::visitBinaryExpr(shared_ptr<Binary<Object>> expr) {
    //llvm to generate IR for binary
    if (expr->operation.lexeme == "+") {
        GEN_BINARY_OP(CreateAdd, "tmpadd");
    } else if (expr->operation.lexeme == "-") {
        GEN_BINARY_OP(CreateSub, "tmpsub");
    } else if (expr->operation.lexeme == "*") {
        GEN_BINARY_OP(CreateMul, "tmpmul");
    } else if (expr->operation.lexeme == "/") {
        GEN_BINARY_OP(CreateSDiv, "tmpdiv");
    }
    // Unsigned comparison
    else if (expr->operation.lexeme == ">") {
        GEN_BINARY_OP(CreateICmpUGT, "tmpcmp");
    } else if (expr->operation.lexeme == "<") {
        GEN_BINARY_OP(CreateICmpULT, "tmpcmp");
    } else if (expr->operation.lexeme == "==") {
        GEN_BINARY_OP(CreateICmpEQ, "tmpcmp");
    } else if (expr->operation.lexeme == "!=") {
        GEN_BINARY_OP(CreateICmpNE, "tmpcmp");
    } else if (expr->operation.lexeme == ">=") {
        GEN_BINARY_OP(CreateICmpUGE, "tmpcmp");
    } else if (expr->operation.lexeme == "<=") {
        GEN_BINARY_OP(CreateICmpULE, "tmpcmp");
    }
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitGroupingExpr(shared_ptr<Grouping<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitUnaryExpr(shared_ptr<Unary<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitVariableExpr(shared_ptr<Variable<Object>> expr) {
    // use the declared variable
    string varName = expr->name.lexeme;
    auto value = environment->lookup(varName);
    //local variable
    if (auto localVar = llvm::dyn_cast<llvm::AllocaInst>(value)) {
        auto var = builder->CreateLoad(localVar->getAllocatedType(), localVar, varName.c_str());
        return Object::make_llvmval_obj(var);
    }

    // global variable
    else if (auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
        auto var = builder->CreateLoad(globalVar->getInitializer()->getType(), globalVar, varName.c_str());
        return Object::make_llvmval_obj(var);
    } else {
        return Object::make_llvmval_obj(value);
    }
    // return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitLogicalExpr(shared_ptr<Logical<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitIncrementExpr(shared_ptr<Increment<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitDecrementExpr(shared_ptr<Decrement<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitListExpr(shared_ptr<List<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitSubscriptExpr(shared_ptr<Subscript<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitCallExpr(shared_ptr<Call<Object>> expr) {
    auto callable = evaluate(expr->callee);
    std::vector<llvm::Value *> args{};
    for (auto arg: expr->arguments) {
        args.push_back(evaluate(arg));
    }

    auto fn = (llvm::Function *) callable;
    auto val = builder->CreateCall(fn, args);
    return Object::make_llvmval_obj(val);
}

Object LoxVM::visitGetExpr(shared_ptr<Get<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitSetExpr(shared_ptr<Set<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitThisExpr(shared_ptr<This<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

Object LoxVM::visitSuperExpr(shared_ptr<Super<Object>> expr) {
    return Object::make_llvmval_obj(builder->getInt32(0));
}

void LoxVM::visitExpressionStmt(const Expression &stmt) {
    lastValue = evaluate(stmt.expression);
    Values.push_back(lastValue);
}

void LoxVM::visitPrintStmt(const Print &stmt) {
    auto expr = std::dynamic_pointer_cast<Call<Object>>(stmt.expression);
    auto printFn = module->getFunction("printf");
    // auto str = builder->CreateGlobalStringPtr("Hello, World!");
    std::vector<llvm::Value *> args;
    for (auto arg: expr->arguments) {
        args.push_back(evaluate(arg));
    }

    lastValue = builder->CreateCall(printFn, args);
    Values.push_back(lastValue);
}

void LoxVM::visitVarStmt(const Var &stmt) {
    // special case for class fields,
    // which are already defined in class info allocation
    if (cls != nullptr) {
        lastValue = builder->getInt32(0);
        return;
    }
    auto varName = stmt.name.lexeme;
    if (stmt.initializer != nullptr) {
        // auto varNameDecl = stmt.initializer;
        // auto env = globalEnv;
        auto init = evaluate(stmt.initializer);
        auto varTy = excrateVarType(stmt.typeName);
        auto varBinding = allocVar(varName, varTy, environment);
        lastValue = builder->CreateStore(init, varBinding);
        // lastValue = createGlobalVariable(varName, (llvm::Constant *) init)->getInitializer();
    } else {
        // global variable without initializer to be assigned 0
        lastValue = createGlobalVariable(varName, builder->getInt32(0))->getInitializer();
    }
    Values.push_back(lastValue);
}

void LoxVM::visitBlockStmt(const Block &stmt) {
    shared_ptr<Environment> env = std::make_shared<Environment>(environment, std::map<std::string, llvm::Value *>{});
    executeBlock(stmt.statements, env);
}

void LoxVM::visitClassStmt(const Class &stmt) {
    auto className = stmt.name.lexeme;
    auto parent = stmt.superclass == nullptr ? nullptr : getClassByName(stmt.superclass->name.lexeme);

    // compile class
    cls = llvm::StructType::create(*ctx, className);
    //! need to delete
    module->getOrInsertGlobal(className, cls);
    // super class data always sit at the beginning
    if (parent != nullptr) {
        inheritClass(cls, parent);
    } else {
        // add class info
        classMap_[className] = {cls, parent, /*field*/ {}, /*method*/ {}};
    }
    // populate the class with fields and methods
    buildClassInfo(cls, stmt, environment);

    // compile the class body
    for (auto method: stmt.methods) {
        execute(method);
    }


    // reset class after compilation, so normal function does not pick the class name prefix
    cls = nullptr;

    lastValue = builder->getInt32(0);
}

void LoxVM::visitIfStmt(const If &stmt) {
    auto conditon = evaluate(stmt.main_branch.condition);

    auto thenBlock = createBB("then", fn);
    // else, if-end block to handle nested if expression
    auto elseBlock = createBB("else");
    auto ifEndBlock = createBB("ifend");

    // conditon branch
    builder->CreateCondBr(conditon, thenBlock, elseBlock);
    // then branch
    builder->SetInsertPoint(thenBlock);
    execute(stmt.main_branch.statement);
    auto thenRes = lastValue;
    builder->CreateBr(ifEndBlock);
    // restore block to handle nested if expression
    // which is needed for phi instruction
    thenBlock = builder->GetInsertBlock();
    // else branch
    // append block to function
    elseBlock->insertInto(fn);// in llvm 17
    // fn->getBasicBlockList().push_back(elseBlock); in llvm 14
    builder->SetInsertPoint(elseBlock);
    llvm::Value *elseRes = lastValue;
    if (stmt.else_branch != nullptr) {
        execute(stmt.else_branch);
        elseRes = lastValue;
    } else {
        // If there's no else block, create a branch from elseBlock to ifEndBlock directly
        // and set the default elseRes value.
        elseRes = builder->getInt32(0);// Set default value for elseRes
    }

    builder->CreateBr(ifEndBlock);
    // same for else block
    elseBlock = builder->GetInsertBlock();
    // endif
    ifEndBlock->insertInto(fn);
    // fn->getBasicBlockList().push_back(ifEndBlock);
    builder->SetInsertPoint(ifEndBlock);

    // result of if expression is phi
    auto phi = builder->CreatePHI(thenRes->getType(), 2, "tmpif");
    phi->addIncoming(thenRes, thenBlock);
    phi->addIncoming(elseRes, elseBlock);
    lastValue = phi;
}
void LoxVM::visitWhileStmt(const While &stmt) {
    // condition
    auto condBlock = createBB("cond", fn);
    builder->CreateBr(condBlock);

    // Body:while end loop
    auto bodyBlock = createBB("body");
    auto loopEndBlock = createBB("loopend");

    // compile while
    builder->SetInsertPoint(condBlock);
    auto condition = evaluate(stmt.condition);

    // condition branch
    builder->CreateCondBr(condition, bodyBlock, loopEndBlock);

    // body
    bodyBlock->insertInto(fn);
    builder->SetInsertPoint(bodyBlock);
    execute(stmt.body);

    // jump to condition block unconditionally
    builder->CreateBr(condBlock);

    // end while
    loopEndBlock->insertInto(fn);
    builder->SetInsertPoint(loopEndBlock);
    lastValue = builder->getInt32(0);
}

void LoxVM::visitFunctionStmt(shared_ptr<Function> stmt) {
    auto fnName = stmt->functionName.lexeme;
    auto params = stmt->params;
    auto funBody = stmt->body;

    // save current function
    auto prevFn = fn;
    auto prevBlock = builder->GetInsertBlock();
    auto prevEnv = environment;
    // override fn to compile body
    auto newFn = createFunction(fnName, excrateFunType(stmt), environment);
    fn = newFn;

    // set parameter name
    auto idx = 0;
    environment = std::make_shared<Environment>(environment, std::map<std::string, llvm::Value *>{});

    for (auto &arg: fn->args()) {
        auto param = params[idx++];
        auto argName = param.first.lexeme;
        arg.setName(argName);

        // allocate a local variable prr argument to make sure arguments mutable
        auto argBinding = allocVar(argName, arg.getType(), environment);
        builder->CreateStore(&arg, argBinding);
    }

    gen(funBody);
    // builder->CreateRet(lastValue);

    // restore previous env after compiling
    builder->SetInsertPoint(prevBlock);
    fn = prevFn;
    environment = prevEnv;
    lastValue = newFn;
}

void LoxVM::visitReturnStmt(const Return &stmt) {
    auto returnVal = evaluate(stmt.value);
    builder->CreateRet(returnVal);
}

void LoxVM::visitBreakStmt(const Break &stmt) {}

void LoxVM::visitContinueStmt(const Continue &stmt) {}

LoxVM::EnvironmentGuard::EnvironmentGuard(
    LoxVM &vm, std::shared_ptr<Environment> enclosing_env
)
    : vm{vm}, previous_env{vm.environment} {
    vm.environment = std::move(enclosing_env);
}

LoxVM::EnvironmentGuard::~EnvironmentGuard() {
    vm.environment = std::move(previous_env);
}
