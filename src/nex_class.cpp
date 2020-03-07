#include "nex_class.hpp"
#include "nex_function.hpp"

#include <map>

namespace nex {
NexClass::NexClass(std::wstring const& name,
                   std::shared_ptr<NexClass> superclass,
                   Fields  const& fields,
                   Methods const& methods)
    : m_name(name)
    , m_superclass(superclass)
    , m_fields(fields)
    , m_methods(methods)
{}

size_t NexClass::arity() const
{
    auto initializer = findMethod(L"init");
    if (!initializer) {
        return 0;
    }
    return initializer->arity();
}

std::any NexClass::call(Interpreter* interp, std::vector<std::any> arguments)
{
    std::map<std::wstring, std::any> instanceFields;
    for (auto& [name, let] : m_fields) {
        interp->visitLetStmt(let.get());
        instanceFields[name] = interp->getEnv()->get(let->m_name);
    }

    auto instance = std::make_shared<NexInstance>(this, instanceFields);

    auto initializer = findMethod(L"init");
    if (initializer) {
        initializer->bind(instance)->call(interp, arguments);
    }

    return std::make_any<std::shared_ptr<NexInstance>>(instance);
}

std::wstring NexClass::to_string() const {
    return L"<class '" + m_name + L"'>";
}

std::wstring NexClass::name() const {
    return m_name;
}

std::shared_ptr<NexFunction> NexClass::findMethod(std::wstring const& name) const
{
    if (m_methods.count(name)) {
        return m_methods.at(name);
    }

    if (m_superclass) {
        return m_superclass->findMethod(name);
    }

    return nullptr;
}

}