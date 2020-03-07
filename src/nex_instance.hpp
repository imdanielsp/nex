#ifndef NEX_NEX_INSTANCE_HPP
#define NEX_NEX_INSTANCE_HPP

#include "nex_token.hpp"

#include <string>
#include <map>
#include <memory>

namespace nex {
class NexClass;

class NexInstance
{
public:
    NexInstance() = default;
    NexInstance(NexClass* pKlass, std::map<std::wstring, std::any> fields);

    ~NexInstance() = default;

    std::wstring to_string();

    std::any get(Token const& name);
    std::any set(Token const& name, std::any const& value);

private:
    NexClass* m_pKlass;
    std::map<std::wstring, std::any> m_fields;
};

}
#endif