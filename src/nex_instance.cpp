#include "nex_instance.hpp"
#include "nex_class.hpp"
#include "nex_runtime_error.hpp"

namespace nex {

NexInstance::NexInstance(NexClass* pKlass,
                         std::map<std::wstring, std::any> fields)
    : m_pKlass(pKlass)
    , m_fields(fields)
{}

std::wstring NexInstance::to_string()
{
    return L"<'" + m_pKlass->m_name + L"' instance>";
}

std::any NexInstance::get(Token const& name)
{
    if (m_fields.count(name.m_lexeme)) {
        return m_fields[name.m_lexeme];
    }

    if (auto pMethod = m_pKlass->findMethod(name.m_lexeme)) {
        std::shared_ptr<NexInstance> tthis(this, [](NexInstance*){});
        return std::dynamic_pointer_cast<NexCallable>(pMethod->bind(tthis));
    }

    throw NexRunTimeError(name,
        m_pKlass->m_name + L" object has not property '" + name.m_lexeme + L"'");
}

std::any NexInstance::set(Token const& name, std::any const& value)
{
    if (m_fields.count(name.m_lexeme)) {
        m_fields[name.m_lexeme] = value;
        return value;
    }

    throw NexRunTimeError(name,
        m_pKlass->m_name + L" object has not property '" + name.m_lexeme + L"'");
}

}
