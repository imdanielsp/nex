#ifndef NEX_RUNTIME_ERROR_HPP
#define NEX_RUNTIME_ERROR_HPP

#include "nex_token.hpp"
#include <exception>
#include <string>

namespace nex {

class NexRunTimeError : public std::runtime_error
{
public:
    NexRunTimeError(const Token& op, const std::wstring& s)
        : std::runtime_error("")
        , m_op(op)
        , m_str(s)
    {}

    virtual ~NexRunTimeError() = default;

    inline std::wstring msg() const
    {
        return m_str;
    }

    const Token& m_op;
    std::wstring m_str;
};

}

#endif
