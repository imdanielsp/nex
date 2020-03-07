#ifndef NEX_FUNCTION_HPP
#define NEX_FUNCTION_HPP

#include "nex_stmt.hpp"
#include "nex_callable.hpp"
#include "nex_environment.hpp"
#include "nex_return.hpp"
#include "nex_instance.hpp"
#include "nex_interpreter.hpp"

#include <any>
#include <string>
#include <vector>
#include <memory>

namespace nex {

class NexFunction : public NexCallable
{
public:
    NexFunction(const stmt::Function& declaration,
                std::shared_ptr<Environment> closure,
                bool bIsInitializer)
        : m_declaration(declaration)
        , m_pClosure(closure)
        , m_bIsInitializer(bIsInitializer)
    {}

    virtual ~NexFunction() = default;

    inline size_t arity() const override
    {
        return m_declaration.m_params.size();
    }

    inline std::any call(Interpreter* interp, std::vector<std::any> arguments) override
    {
        auto localEnv =
            std::make_shared<Environment>(L"<func " + m_declaration.m_name.m_lexeme + L">");
        localEnv->copy(m_pClosure);

        for (size_t idx = 0; idx < m_declaration.m_params.size(); idx++) {
            localEnv->define(m_declaration.m_params.at(idx), arguments.at(idx));
        }

        localEnv->dump();

        try {
            interp->executeBlock(m_declaration.m_body, localEnv);
        } catch (const NexReturn& e) {
            if (m_bIsInitializer) {
                return m_pClosure->getAt(0, L"this");
            }
            return e.m_value;
        }

        if (m_bIsInitializer) {
            return m_pClosure->getAt(0, L"this");
        }

        return nullptr;
    }

    inline std::wstring to_string() const override
    {
        return L"<func '" + m_declaration.m_name.m_lexeme + L"'>";
    }

    inline std::shared_ptr<NexFunction> bind(std::shared_ptr<NexInstance> instance)
    {
        auto env = std::make_shared<Environment>(m_declaration.m_name.m_lexeme);
        env->copy(m_pClosure);
        Token tthis(THIS, L"this", nullptr, 0);
        env->define(tthis, instance);
        return std::make_shared<NexFunction>(m_declaration, env, m_bIsInitializer);
    }

    inline std::wstring name() const override {
        return m_declaration.m_name.m_lexeme;
    }

private:
    const stmt::Function& m_declaration;
    std::shared_ptr<Environment> m_pClosure;
    bool m_bIsInitializer;
};

}

#endif