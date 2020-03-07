#ifndef NEX_ENVIRONMENT_HPP
#define NEX_ENVIRONMENT_HPP

#include "nex_token.hpp"

#include <iostream>
#include <map>
#include <string>
#include <any>
#include <memory>

namespace nex {

class Environment final {
public:
    Environment(const std::wstring& name)
        : m_name(name)
        , m_values()
        , m_pEnclosing(nullptr)
    {}

    ~Environment() = default;

    void copy(std::shared_ptr<Environment> pSrc);

    void assign(const Token& name, std::any value);

    void assignAt(size_t distance, const Token& name, std::any value);

    void define(const Token& name, std::any value);

    std::any get(const Token& name);

    std::any getAt(size_t distance, std::wstring name);

    Environment* ancestor(size_t distance);

    void dump() const
    {
#if defined(DBG_ENVIRONMENT)
        std::wcout << "------ START -------" << std::endl;
        std::wcout << "In " << m_name << " @ " << this << std::endl;
        for (auto& [key, value] : m_values) {
            if (auto val = std::any_cast<double>(&value)) {
                std::wcout << key << L" : " << *val << " @ " << &value << std::endl;
            }
            else {
                std::wcout << key << L" : " << value.type().name() << " @ " << &value << std::endl;
            }
        }

        if (m_pEnclosing) {
            m_pEnclosing->dump();
        }
        std::wcout << "------ END -------" << std::endl;
#endif
    }

    std::wstring m_name;
    std::map<std::wstring, std::any> m_values;
    std::shared_ptr<Environment> m_pEnclosing;
};

}

#endif