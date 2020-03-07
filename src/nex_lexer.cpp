#include "nex_lexer.hpp"
#include "nex_diag.hpp"

#include <iterator>
#include <cctype>

namespace nex {

namespace {
const std::map<std::wstring, TokenType> g_keywords {
    // Keywords
    { L"and", AND },
    { L"class", CLASS },
    { L"else", ELSE },
    { L"false", FALSE },
    { L"for", FOR },
    { L"func", FUNC },
    { L"if", IF },
    { L"nil", NIL },
    { L"or", OR },
    { L"print", PRINT },
    { L"input", INPUT },
    { L"ret", RET },
    { L"super", SUPER },
    { L"this", THIS },
    { L"true", TRUE },
    { L"const", CONST },
    { L"let", LET },
    { L"while", WHILE },
    { L"typeof", TYPE_OF },
    { L"extends", EXTENDS },
    // Built-in types
    { L"Void", TYPE_VOID },
    { L"Int", TYPE_INT },
    { L"String", TYPE_STRING }
};
}

Lexer::Lexer(std::wistream& is)
    : m_source(std::istreambuf_iterator<wchar_t>(is), {})
    , m_tokens()
    , m_bHadError(false)
    , m_start(0)
    , m_current(0)
    , m_line(1)
{}

std::vector<Token> Lexer::scan()
{
    while (!isAtEnd()) {
        m_start = m_current;
        scanToken();
    }

    m_tokens.push_back(Token(END_OF_FILE, L"", nullptr, m_line));
    return m_tokens;
}

bool Lexer::isAtEnd() const {
    return m_current >= m_source.size();
}

void Lexer::scanToken()
{
    wchar_t c = advance();
    switch (c) {
    case '(': addToken(LEFT_PAREN); break;
    case ')': addToken(RIGHT_PAREN); break;
    case '{': addToken(LEFT_BRACE); break;
    case '}': addToken(RIGHT_BRACE); break;
    case ',': addToken(COMMA); break;
    case '.': addToken(DOT); break;
    case '+': addToken(PLUS); break;
    case ':': addToken(COLON); break;
    case ';': addToken(SEMICOLON); break;
    case '*': addToken(STAR); break;
    // Implementation notes:
    // Lexer::match could be Lexer::match(next, true val, false, val) -> val
    case '-': addToken(match('>') ? ARROW : MINUS); break;
    case '!': addToken(match('=') ? BANG_EQUAL : BANG); break;
    case '=': addToken(match('=') ? EQUAL_EQUAL : EQUAL); break;
    case '<': addToken(match('=') ? LESS_EQUAL : LESS); break;
    case '>': addToken(match('=') ? GREATER_EQUAL : GREATER); break;
    case '?': addToken(QUESTION_MARK); break;
    case '/':
        if (match('/')) {
            // It is a comment, skip until the end of the line
            while (peek() != '\n' && !isAtEnd()) {
                advance();
            }
        }
        else {
            addToken(SLASH);
        }
        break;
    // Ignore whitespace
    case ' ':
    case '\r':
    case '\t':
        break;
    case '\n':
        m_line++;
        break;
    case '"': handleString(); break;
    default:
        if (std::isdigit(c)) {
            handleNumber();
        }
        else if (isAlpha(c)) {
            handleIdentifier();
        }
        else {
            m_bHadError = true;
            ::nex::error(m_line, L"Unexpected wchar_tacter: " + std::wstring(1, c));
        }
    }
}

wchar_t Lexer::advance()
{
    m_current++;
    return m_source.at(m_current - 1);
}

void Lexer::addToken(TokenType t)
{
    addToken(t, nullptr);
}

void Lexer::addToken(TokenType t, std::any literal)
{
    auto text = m_source.substr(m_start, m_current - m_start);
    m_tokens.push_back(Token(t, text, literal, m_line));
}

bool Lexer::match(wchar_t expected)
{
    if (isAtEnd()) {
        return false;
    }

    if (m_source.at(m_current) != expected) {
        return false;
    }

    m_current++;
    return true;
}

wchar_t Lexer::peek() const
{
    if (isAtEnd()) {
        return '\0';
    }
    return m_source.at(m_current);
}

wchar_t Lexer::peekNext() const
{
    if (m_current + 1 >= m_source.size()) {
        return '\0';
    }
    return m_source.at(m_current + 1);
}

void Lexer::handleString()
{
    auto size = 0;
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') m_line++;
        advance();
        size++;
    }

    // Unterminated string
    if (isAtEnd()) {
        ::nex::error(m_line, L"Unterminated string.");
        return;
    }

    // The closing "
    advance();

    auto value = m_source.substr(m_start + 1, size);
    addToken(STRING, value);
}

void Lexer::handleNumber()
{
    while (std::isdigit(peek())) {
        advance();
    }

    // Look for a fractional part
    if (peek() == '.' && std::isdigit(peekNext())) {
        // Consume the "."
        advance();

        while (std::isdigit(peek())) {
            advance();
        }
    }

    addToken(NUMBER, std::stod(m_source.substr(m_start, m_current - m_start)));
}

void Lexer::handleIdentifier()
{
    while (isAlphaNumeric(peek())) {
        advance();
    }

    TokenType type = IDENTIFIER;
    auto text = m_source.substr(m_start, m_current - m_start);
    if (g_keywords.count(text) != 0) {
        type = g_keywords.find(text)->second;
    }

    addToken(type);
}

bool Lexer::isAlpha(wchar_t c) const
{
    return (c >= 'a' && c < 'z') ||
           (c >= 'A' && c < 'Z') ||
            c == '_';
}

bool Lexer::isAlphaNumeric(wchar_t c) const
{
    return std::isdigit(c) || isAlpha(c);
}

}