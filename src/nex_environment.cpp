#include "nex_environment.hpp"
#include "nex_runtime_error.hpp"

#include <algorithm>

namespace nex {

void Environment::copy(std::shared_ptr<Environment> pSrc)
{
    if (!m_pEnclosing) {
        m_pEnclosing = std::make_shared<Environment>(pSrc->m_name + L"'");
        m_pEnclosing->m_values.insert(std::begin(pSrc->m_values),
                                      std::end(pSrc->m_values));
        if (pSrc->m_pEnclosing) {
            m_pEnclosing->copy(pSrc->m_pEnclosing);
        }
    }
}

void Environment::assign(const Token& name, std::any value)
{
    if (m_values.count(name.m_lexeme)) {
        m_values[name.m_lexeme] = value;
        return;
    }

    if (m_pEnclosing != nullptr) {
        m_pEnclosing->assign(name, value);
        return;
    }

    throw NexRunTimeError(name, L"Undefined symbol '" + name.m_lexeme + L"'.");
}

void Environment::assignAt(size_t distance, const Token& name, std::any value)
{
    ancestor(distance)->m_values[name.m_lexeme] = value;
}

void Environment::define(const Token& name, std::any value)
{
    if (m_values.count(name.m_lexeme) == 0) {
        m_values[name.m_lexeme] = value;
    }
    else {
        throw NexRunTimeError(name, L"Symbol '" + name.m_lexeme + L"' has already been declared");
    }
}

std::any Environment::get(const Token& name)
{
    if (m_values.count(name.m_lexeme)) {
        return m_values[name.m_lexeme];
    }

    if (m_pEnclosing != nullptr) {
        return m_pEnclosing->get(name);
    }

    throw NexRunTimeError(name, L"Undefined symbol '" + name.m_lexeme + L"'.");
}

std::any Environment::getAt(size_t distance, std::wstring name)
{
    return ancestor(distance)->m_values[name];
}

Environment* Environment::ancestor(size_t distance)
{
    auto pEnv = this;
    for (size_t idx = 0; idx < distance; idx++) {
        pEnv = pEnv->m_pEnclosing.get();
    }
    return pEnv;
}

}