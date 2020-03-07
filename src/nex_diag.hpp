#ifndef NEX_DIAG_HPP_
#define NEX_DIAG_HPP_

#include "nex_runtime_error.hpp"

#include <iostream>
#include <string>


namespace nex {
    inline void report(int line, std::wstring where, std::wstring msg) {
        std::wcout << "[line " << line << "] Error " << where << ": " << msg
                  << std::endl;
    }

    inline void error(int line, std::wstring msg) {
        report(line, L"", msg);
    }

    inline void runtimeError(const NexRunTimeError& error)
    {
        std::wcout << error.what()
                   << " [line "
                   << error.m_op.m_line << "] "
                   << error.msg()
                   << std::endl;
    }

}

#endif
