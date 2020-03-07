#ifndef NEX_RUNTIME_HPP
#define NEX_RUNTIME_HPP

#include "nex_callable.hpp"

#include <chrono>
#include <string>

namespace nex::runtime {

// Native function table (identifier and symbol)
#define NATIVE_FN_LIST \
    EMIT_NATIVE_FN(Token(IDENTIFIER, L"clock", nullptr, 0), SystemClock())

class SystemClock : public NexCallable
{
public:
    SystemClock() = default;
    virtual ~SystemClock() = default;

    inline
    std::any call(Interpreter* interp, std::vector<std::any> arguments) override
    {
        (void) interp;
        (void) arguments;

        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        return static_cast<double>(t);
    }

    inline size_t arity() const override
    {
        return 0;
    }

    inline std::wstring to_string() const override
    {
        return L"<native func 'clock'>";
    }

    inline std::wstring name() const override {
        return L"clock";
    }
};

}
#endif