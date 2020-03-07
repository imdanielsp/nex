#ifndef NEX_NEX_CLASS_HPP
#define NEX_NEX_CLASS_HPP

#include "nex_callable.hpp"
#include "nex_instance.hpp"
#include "nex_function.hpp"
#include "nex_stmt.hpp"

#include <string>
#include <memory>

namespace nex {

using namespace nex::ast;

class Interpreter;

class NexClass : public NexCallable
{
public:
    using Fields = std::map<std::wstring, std::shared_ptr<stmt::Let>>;
    using Methods = std::map<std::wstring, std::shared_ptr<NexFunction>>;

public:
    NexClass() = default;
    NexClass(std::wstring const& name,
             std::shared_ptr<NexClass> m_superclass,
             Fields const& fields,
             Methods const& methods);

    virtual ~NexClass() = default;

    size_t arity() const override;

    std::any call(Interpreter* interp, std::vector<std::any> arguments) override;

    std::wstring to_string() const override;

    std::wstring name() const override;

    std::shared_ptr<NexFunction> findMethod(std::wstring const& name) const;

    std::wstring m_name;
    std::shared_ptr<NexClass> m_superclass;
    Fields m_fields;
    Methods m_methods;
};
}

#endif