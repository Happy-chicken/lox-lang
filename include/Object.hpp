#include <llvm/IR/Value.h>
#include <memory>
#include <string>
#include <variant>

// #include <xtree>

using std::shared_ptr;
using std::string;

class LoxCallable;
class LoxClass;
class LoxInstance;
class ListInstance;
class LoxList;
//monostate non-valid state in variant in c++17
using Objects = std::variant<std::string, double, bool, std::monostate, shared_ptr<LoxList>, shared_ptr<LoxCallable>, shared_ptr<LoxInstance>, shared_ptr<LoxClass>>;
class Object {
public:
    Objects data;
    string toString();

    llvm::Value *llvmValue = nullptr;
    llvm::Function *llvmFunction = nullptr;

    static Object make_nil_obj();
    static Object make_class_obj(shared_ptr<LoxClass> lox_class_);

    template<typename T>
    static Object make_obj(T data_) {
        Object obj;
        obj.data = data_;
        return obj;
    }

    template<typename T>
    static Object make_instance_obj(T instance_) {
        Object instance_obj;
        instance_obj.data = instance_;
        return instance_obj;
    }

    static Object make_llvmval_obj(llvm::Value *value) {
        Object obj;
        obj.llvmValue = value;
        return obj;
    }

    static Object make_llvmfunc_obj(llvm::Function *func) {
        Object obj;
        obj.llvmFunction = func;
        return obj;
    }
};