#ifndef NEX_RETURN_HPP
#define NEX_RETURN_HPP

#include <any>
#include <exception>

namespace nex {

class NexReturn : public std::runtime_error
{
public:
    explicit NexReturn(const std::any& value)
        : std::runtime_error("")
        , m_value(value)
    {}

    virtual ~NexReturn() = default;

    std::any m_value;
};

}
#endif