#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "RuntimeError.hpp"
#include "Token.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <system_error>
#include <vector>
#define DIE ErrorLogMessage()
namespace Error {
    class ErrorLogMessage : public std::basic_ostringstream<char> {
    public:
        ~ErrorLogMessage() {
            std::cerr << "Fatal error: " << str().c_str() << std::endl;
            exit(EXIT_FAILURE);
        }
    };

    struct ErrorInfo {
        const unsigned int line;
        const std::string where;
        const std::string message;

        ErrorInfo(unsigned int line, std::string where, std::string message)
            : line{line}, where{std::move(where)}, message{std::move(message)} {
        }
    };

    void report() noexcept;

    void addRuntimeError(const RuntimeError &error) noexcept;

    void addError(unsigned int line, std::string where, std::string message) noexcept;

    void addError(const Token &token, std::string message) noexcept;

    extern bool hadError;
    extern bool hadRuntimeError;
    extern std::vector<ErrorInfo> exceptionList;
}// namespace Error

#endif// LOGGER_HPP