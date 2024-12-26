#include "./IRgenerator.hpp"
#include <llvm/IR/GlobalVariable.h>
#include <memory>
class LoxVM : public Visitor<Object>,
              public Visitor_Stmt,
              public std::enable_shared_from_this<LoxVM> {
public:
    LoxVM() {
        moduleInit();
        setupExternalFunctions();
    };
    ~LoxVM() = default;

    void exec(vector<shared_ptr<Stmt>> &statements);

private:
    void compile(vector<shared_ptr<Stmt>> &statements);
    void saveModuleToFile(const std::string &fileName);
    void moduleInit();                                                                         // init module and context
    void setupExternalFunctions();                                                             // setup external functions
    llvm::Function *createFunction(const std::string &fnName, llvm::FunctionType *fnType);     // create a function
    llvm::Function *createFunctionProto(const std::string &fnName, llvm::FunctionType *fnType);// create a function prototype
    llvm::GlobalVariable *createGlobalVariable(const std::string &name, llvm::Constant *init); // create a global variable
    llvm::Value *gen(vector<shared_ptr<Stmt>> &statements);                                    // generate IR
    llvm::BasicBlock *createBB(const std::string &name, llvm::Function *fn);                   // create a basic block
    void createFunctionBlock(llvm::Function *fn);                                              // create a function block

    llvm::Value *lastValue = nullptr;          // last value generated
    llvm::Function *fn;                        // current compiling function
    std::unique_ptr<llvm::LLVMContext> ctx;    // container for modules and other LLVM objects
    std::unique_ptr<llvm::Module> module;      // container for functions and global variables
    std::unique_ptr<llvm::IRBuilder<>> builder;// helps to generate IR

    //runner functon
    // Object evaluate(shared_ptr<Expr<Object>> expr);
    void execute(shared_ptr<Stmt> stmt);
    void executeBlock(vector<shared_ptr<Stmt>> statements /*environment*/);

    // generate IR for statements
    void codegenerate(vector<shared_ptr<Stmt>> &statements);
    void codegenerate(shared_ptr<Stmt> stmt);
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