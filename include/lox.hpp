#ifndef LOX_HPP_
#define LOX_HPP_

#include "RuntimeError.hpp"
#include <string>

using std::string;

class lox {
public:
    static int runScript(int argc, char const *argv[]);

private:
    static void runFile(string path);
    static void buildFile(string path);

    static void run(string source);
    static void build(string source);

    static void runPrompt();
};

#endif// LOX_HPP_
