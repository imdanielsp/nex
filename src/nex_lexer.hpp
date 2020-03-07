#ifndef NEX_LEXER_H_
#define NEX_LEXER_H_

#include <istream>
#include <vector>
#include <any>
#include <map>

#include "nex_token.hpp"

namespace nex {

class Lexer final
{
public:
    explicit Lexer(std::wistream& is);
    ~Lexer() = default;

    std::vector<Token> scan();

    inline bool error() const noexcept { return m_bHadError; }

private:
    bool isAtEnd() const;
    void scanToken();
    void addToken(TokenType t);
    void addToken(TokenType t, std::any literal);
    wchar_t advance();
    bool match(wchar_t next);
    wchar_t peek() const;
    wchar_t peekNext() const;

    void handleString();
    void handleNumber();
    void handleIdentifier();

    bool isAlpha(wchar_t c) const;
    bool isAlphaNumeric(wchar_t c) const;

private:
    std::wstring m_source;
    std::vector<Token> m_tokens;
    bool m_bHadError;
    size_t m_start;
    size_t m_current;
    size_t m_line;
};

}

#endif
