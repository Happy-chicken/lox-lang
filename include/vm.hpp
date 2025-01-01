#include "./Environment.hpp"
#include "./IRgenerator.hpp"
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilderFolder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <vector>

using Env = std::shared_ptr<Environment>;

// Generic binary operator:
#define GEN_BINARY_OP(Op, varName)                    \
    do {                                              \
        auto left = evaluate(expr->left);             \
        auto right = evaluate(expr->right);           \
        auto val = builder->Op(left, right, varName); \
        return Object::make_llvmval_obj(val);         \
    } while (false)

// class information
struct ClassInfo {
    llvm::StructType *cls;
    llvm::StructType *parent;
    std::map<std::string, llvm::Type *> fieldsMap;
    std::map<std::string, llvm::Function *> methodsMap;
};

class LoxVM : public Visitor<Object>,
              public Visitor_Stmt,
              public std::enable_shared_from_this<LoxVM> {
public:
    LoxVM() {
        moduleInit();
        setupExternalFunctions();
        setupGlobalEnvironment();
        setupTargetTriple();
    };
    ~LoxVM() = default;

    class EnvironmentGuard {
    public:
        EnvironmentGuard(LoxVM &vm, Env env);

        ~EnvironmentGuard();

    private:
        LoxVM &vm;
        std::shared_ptr<Environment> previous_env;
    };

    void exec(vector<shared_ptr<Stmt>> &statements);

private:
    void compile(vector<shared_ptr<Stmt>> &statements);
    void saveModuleToFile(const std::string &fileName);
    void moduleInit();                                                                                  // init module and context
    void setupExternalFunctions();                                                                      // setup external functions
    void setupGlobalEnvironment();                                                                      // setup global environment
    void setupTargetTriple();                                                                           // setup target triple
    llvm::Function *createFunction(const std::string &fnName, llvm::FunctionType *fnType, Env env);     // create a function
    llvm::Function *createFunctionProto(const std::string &fnName, llvm::FunctionType *fnType, Env env);// create a function prototype
    llvm::Value *allocVar(const std::string &name, llvm::Type *type, Env env);                          // allocate a variable
    llvm::GlobalVariable *createGlobalVariable(const std::string &name, llvm::Constant *init);          // create a global variable
    llvm::Value *gen(vector<shared_ptr<Stmt>> &statements);                                             // generate IR
    llvm::BasicBlock *createBB(const std::string &name, llvm::Function *fn);                            // create a basic block
    void createFunctionBlock(llvm::Function *fn);                                                       // create a function block

    llvm::Type *excrateVarType(std::shared_ptr<Expr<Object>> expr);                                 // extract type from expression
    llvm::Type *excrateVarType(const string &typeName);                                             // extract type from string
    bool hasReturnType(shared_ptr<Stmt> stmt);                                                      // check if a function has return type
    llvm::FunctionType *excrateFunType(shared_ptr<Function> stmt);                                  // extract function type
    llvm::Value *createInstance(shared_ptr<Call<Object>> expr, Env env, const std::string &varName);// create instance

    llvm::StructType *getClassByName(const std::string &name);             // get class by name
    void inheritClass(llvm::StructType *cls, llvm::StructType *parent);    // inherit parent class field
    void buildClassInfo(llvm::StructType *cls, const Class &stmt, Env env);// build class info
    void buildClassBody(llvm::StructType *cls);                            // build class body


    static llvm::Value *lastValue;                 // last value generated
    std::vector<llvm::Value *> Values;             // all IR values
    Env globalEnv;                                 // global environment
    Env &environment = globalEnv;                  // current env
    llvm::Function *fn;                            // current compiling function
    std::unique_ptr<llvm::LLVMContext> ctx;        // container for modules and other LLVM objects
    std::unique_ptr<llvm::Module> module;          // container for functions and global variables
    std::unique_ptr<llvm::IRBuilder<>> varsBuilder;// this builder always prepends to the beginning of the function entry block
    std::unique_ptr<llvm::IRBuilder<>> builder;    // enter at the end of the function entry block
    llvm::StructType *cls = nullptr;               // current compiling class type
    std::map<std::string, ClassInfo> classMap_;    // class map

    //runner functon

    // generate IR for statements
    void codegenerate(vector<shared_ptr<Stmt>> &statements);

    void executeBlock(vector<shared_ptr<Stmt>> statements, Env env);
    void execute(shared_ptr<Stmt> stmt);
    llvm::Value *evaluate(shared_ptr<Expr<Object>> expr);

    // IR generator visitor
    Object visitLiteralExpr(shared_ptr<Literal<Object>> expr);
    Object visitAssignExpr(shared_ptr<Assign<Object>> expr);
    Object visitBinaryExpr(shared_ptr<Binary<Object>> expr);
    Object visitGroupingExpr(shared_ptr<Grouping<Object>> expr);
    Object visitUnaryExpr(shared_ptr<Unary<Object>> expr);
    Object visitVariableExpr(shared_ptr<Variable<Object>> expr);
    Object visitLogicalExpr(shared_ptr<Logical<Object>> expr);

    Object visitIncrementExpr(shared_ptr<Increment<Object>> expr);
    Object visitDecrementExpr(shared_ptr<Decrement<Object>> expr);
    Object visitListExpr(shared_ptr<List<Object>> expr);
    Object visitSubscriptExpr(shared_ptr<Subscript<Object>> expr);

    Object visitCallExpr(shared_ptr<Call<Object>> expr);
    Object visitGetExpr(shared_ptr<Get<Object>> expr);
    Object visitSetExpr(shared_ptr<Set<Object>> expr);
    Object visitThisExpr(shared_ptr<This<Object>> expr);
    Object visitSuperExpr(shared_ptr<Super<Object>> expr);

    void visitExpressionStmt(const Expression &stmt);
    void visitPrintStmt(const Print &stmt);
    void visitVarStmt(const Var &stmt);
    void visitBlockStmt(const Block &stmt);
    void visitClassStmt(const Class &stmt);
    void visitIfStmt(const If &stmt);
    void visitWhileStmt(const While &stmt);
    void visitFunctionStmt(shared_ptr<Function> stmt);
    void visitReturnStmt(const Return &stmt);
    void visitBreakStmt(const Break &stmt);
    void visitContinueStmt(const Continue &stmt);
};