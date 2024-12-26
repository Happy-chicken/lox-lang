#include "./include/lox.hpp"

// #include "./include/IRgenerator.hpp"
#include "./include/Interpreter.hpp"
#include "./include/Logger.hpp"
#include "./include/Parser.hpp"
#include "./include/Resolver.hpp"
#include "./include/RuntimeError.hpp"
#include "./include/Scanner.hpp"
#include "./include/vm.hpp"
#include "include/Token.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
using std::cout;
using std::endl;
using std::ifstream;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::vector;

int lox::runScript(int argc, const char *argv[]) {
    if (argc > 3) {
        printf("Usage: main run [script] \n");
    } else if (argc == 3) {
        if (string(argv[1]) == "run") {
            runFile(argv[2]);
        } else if (string(argv[1]) == "build") {
            buildFile(argv[2]);
        } else {
            printf("Use run or build \n");
        }

    } else {
        runPrompt();
    }
    return 0;
}

std::string readFile(std::string_view filename) {
    std::ifstream file{filename.data(), std::ios::ate};
    if (!file) {
        std::cerr << "Failed to open file " << filename.data() << '\n';
        std::exit(74);// I/O error
    }
    std::string buffer;
    buffer.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), buffer.size());
    file.close();

    return buffer;
}

void lox::runFile(string path) {
    std::string source = readFile(path);
    run(source);

    if (Error::hadError)
        exit(65);
    if (Error::hadRuntimeError)
        exit(70);
}

void lox::buildFile(string path) {
    std::string source = readFile(path);
    build(source);
    // printf("build file, generate llvm IR\n");
}
void lox::runPrompt() {
    string input;
    while (1) {
        std::cout << "> ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            return;
        }

        run(line);
        Error::hadError = false;
    }
}

void lox::run(string source) {
    shared_ptr<Scanner> scanner = std::make_shared<Scanner>(source);
    vector<Token> tokens = scanner->scanTokens();
    shared_ptr<Parser> parser = std::make_shared<Parser>(tokens);
    vector<shared_ptr<Stmt>> statements = parser->parse();
    // Stop if there was a syntax error.
    if (Error::hadError) {
        Error::report();
        return;
    }

    if (statements.size() == 0) {
        cout << "no value" << endl;
    } else {
        shared_ptr<Interpreter> interpreter = std::make_shared<Interpreter>();
        shared_ptr<Resolver> resolver = std::make_shared<Resolver>(interpreter);
        resolver->resolve(statements);
        // Stop if there was a resolution error.
        if (Error::hadError) {
            Error::report();
            return;
        }

        interpreter->interpret(statements);
        if (Error::hadRuntimeError) {
            Error::report();
            return;
        }
    }
}

void lox::build(string source) {
    shared_ptr<Scanner> scanner = std::make_shared<Scanner>(source);
    vector<Token> tokens = scanner->scanTokens();
    shared_ptr<Parser> parser = std::make_shared<Parser>(tokens);
    vector<shared_ptr<Stmt>> statements = parser->parse();

    shared_ptr<LoxVM> vm = std::make_shared<LoxVM>();
    vm->exec(statements);
}
