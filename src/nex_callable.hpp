#ifndef NEX_CALLABLE_HPP
#define NEX_CALLABLE_HPP

#include <any>
#include <vector>

namespace nex {
class Interpreter;

class NexCallable
{
public:
    virtual size_t arity() const = 0;
    virtual std::any call(Interpreter* interp, std::vector<std::any> arguments) = 0;
    virtual std::wstring to_string() const = 0;
    virtual std::wstring name() const = 0;
};

}

#endif