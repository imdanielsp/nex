#ifndef NEX_TOKEN_HPP_
#define NEX_TOKEN_HPP_

#include <string>
#include <sstream>
#include <ostream>
#include <any>

namespace nex {

#define TOKEN_LIST \
    EMIT_TOKEN(LEFT_PAREN, L"LEFT_PAREN", L"(") \
    EMIT_TOKEN(RIGHT_PAREN, L"RIGHT_PAREN", L")") \
    EMIT_TOKEN(LEFT_BRACE, L"LEFT_BRACE", L"{") \
    EMIT_TOKEN(RIGHT_BRACE, L"RIGHT_BRACE", L"}") \
    EMIT_TOKEN(COMMA, L"COMMA", L",") \
    EMIT_TOKEN(DOT, L"DOT", L".") \
    EMIT_TOKEN(MINUS, L"MINUS", L"-") \
    EMIT_TOKEN(PLUS, L"PLUS", L"+") \
    EMIT_TOKEN(EXTENDS, L"EXTENDS", L"extends") \
    EMIT_TOKEN(ELLIPSIS, L"ELLIPSIS", L"...") \
    EMIT_TOKEN(COLON, L"COLON", L":") \
    EMIT_TOKEN(SEMICOLON, L"SEMICOLON", L";") \
    EMIT_TOKEN(SLASH, L"SLASH", L"/") \
    EMIT_TOKEN(STAR, L"STAR", L"*") \
    EMIT_TOKEN(ARROW, L"ARROW", L"->") \
    EMIT_TOKEN(BANG, L"BANG", L"!") \
    EMIT_TOKEN(BANG_EQUAL, L"BANG_EQUAL", L"!=") \
    EMIT_TOKEN(EQUAL, L"EQUAL", L"=") \
    EMIT_TOKEN(EQUAL_EQUAL, L"EQUAL_EQUAL", L"==") \
    EMIT_TOKEN(GREATER, L"GREATER", L">") \
    EMIT_TOKEN(GREATER_EQUAL, L"GREATER_EQUAL", L">=") \
    EMIT_TOKEN(LESS, L"LESS", L"<") \
    EMIT_TOKEN(LESS_EQUAL, L"LESS_EQUAL", L"<=") \
    EMIT_TOKEN(QUESTION_MARK, L"QUESTION_MARK", L"?") \
    EMIT_TOKEN(IDENTIFIER, L"IDENTIFIER", L"id") \
    EMIT_TOKEN(STRING, L"STRING", L"str") \
    EMIT_TOKEN(NUMBER, L"NUMBER", L"num") \
    EMIT_TOKEN(TYPE_INT, L"TYPE_INT", L"Int") \
    EMIT_TOKEN(TYPE_VOID, L"TYPE_VOID", L"Void") \
    EMIT_TOKEN(TYPE_STRING, L"TYPE_STRING", L"String") \
    EMIT_TOKEN(AND, L"AND", L"and") \
    EMIT_TOKEN(CLASS, L"CLASS", L"class") \
    EMIT_TOKEN(ELSE, L"ELSE", L"else") \
    EMIT_TOKEN(FALSE, L"FALSE", L"false") \
    EMIT_TOKEN(FUNC, L"FUNC", L"func") \
    EMIT_TOKEN(IF, L"IF", L"if") \
    EMIT_TOKEN(NIL, L"NIL", L"nil") \
    EMIT_TOKEN(OR, L"OR", L"or") \
    EMIT_TOKEN(FOR, L"FOR", L"for") \
    EMIT_TOKEN(TEST, L"TEST", L"test") \
    EMIT_TOKEN(PRINT, L"PRINT", L"print") \
    EMIT_TOKEN(INPUT, L"INPUT", L"input") \
    EMIT_TOKEN(READ, L"READ", L"read") \
    EMIT_TOKEN(RET, L"RET", L"ret") \
    EMIT_TOKEN(SUPER, L"SUPER", L"super") \
    EMIT_TOKEN(THIS, L"THIS", L"this") \
    EMIT_TOKEN(TRUE, L"TRUE", L"true") \
    EMIT_TOKEN(CONST, L"CONST", L"const") \
    EMIT_TOKEN(LET, L"LET", L"let") \
    EMIT_TOKEN(WHILE, L"WHILE", L"while") \
    EMIT_TOKEN(TYPE_OF, L"TYPE_OF", L"typeof") \
    EMIT_TOKEN(END_OF_FILE, L"END_OF_FILE", L"EOF")

#define EMIT_TOKEN(id, str, token) id,
enum TokenType {
    TOKEN_LIST
#undef EMIT_TOKEN
    TOKEN_NUM
};

inline std::wstring tokenToStr(TokenType token) {
    assert(token < TOKEN_NUM);
#define EMIT_TOKEN(id, str, token) str,
    const std::wstring tokens[] = { 
        TOKEN_LIST
#undef EMIT_TOKEN
    };
    return tokens[token];
}

class Token final
{
public:
    Token(TokenType type, std::wstring lexeme, std::any literal, int line)
        : m_type(type)
        , m_lexeme(lexeme)
        , m_literal(literal)
        , m_line(line)
    {}

    ~Token() = default;

    std::wstring toString() const
    {
        std::wstringstream wss;
        wss << m_type << " " << m_lexeme << " " << m_literal.type().name() << " " << m_line;
        return wss.str();
    }

    const TokenType m_type;
    const std::wstring m_lexeme;
    std::any m_literal;
    const int m_line;
};


}

#endif
