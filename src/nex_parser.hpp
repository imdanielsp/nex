#ifndef NEX_PARSER_HPP_
#define NEX_PARSER_HPP_

#include "nex_token.hpp"
#include "nex_expr.hpp"
#include "nex_stmt.hpp"

#include <vector>

namespace nex {

using namespace ast::expr;
using namespace ast::stmt;

using ExprPointer = std::shared_ptr<Expr>;
using StmtPointer = std::shared_ptr<Stmt>;

class ParserError : public std::runtime_error {
public:
    ParserError(std::wstring const& msg)
        : std::runtime_error("")
        , m_msg(msg)
    {}

    std::wstring msg() const { return m_msg; }

    virtual ~ParserError() = default;

private:
    std::wstring m_msg;
};

class Parser final {
public:
    Parser(const std::vector<Token>& tokens)
        : m_tokens(tokens)
        , m_current(0)
        , m_bHadError(false)
    {}

    ~Parser() = default;

    std::vector<StmtPointer> parse();

    inline bool error() const {
        return m_bHadError;
    }

private:
    StmtPointer declaration();

    StmtPointer classDeclaration();

    StmtPointer function(const std::wstring& kind);

    StmtPointer letDeclaration();

    StmtPointer statement();

    StmtPointer forStatement();

    StmtPointer ifStatement();

    StmtPointer printStatement();

    StmtPointer returnStatement();

    std::vector<StmtPointer> block();

    StmtPointer expressionStatement();

    StmtPointer whileStatement();

    ExprPointer assignment();

    ExprPointer orExpr();

    ExprPointer andExpr();

    ExprPointer expression();

    ExprPointer equality();

    ExprPointer comparison();

    ExprPointer addition();

    ExprPointer multiplication();

    ExprPointer unary();

    ExprPointer call();

    ExprPointer finishCall(ExprPointer e);

    ExprPointer primary();

    ExprPointer comma(ExprPointer e);

    ExprPointer inputExpr();

    // Helper functions
    bool check(TokenType type);

    Token advance();

    ParserError error(Token token, std::wstring msg);

    template <typename ...Op>
    bool match(Op ...ops)
    {
        std::vector<TokenType> tokens;
        (tokens.push_back(ops), ...);

        for (auto& token : tokens) {
            if (check(token)) {
                advance();
                return true;
            }
        }

        return false;
    }

    inline bool isAtEnd() const {
        return peek().m_type == END_OF_FILE;
    }

    inline Token peek() const {
        return m_tokens.at(m_current);
    }

    inline Token previous() {
        return m_tokens.at(m_current - 1);
    }

    Token consume(TokenType type, std::wstring msg);

    void synchronize();

    void reportError(Token token, std::wstring msg);

private:
    const std::vector<Token>& m_tokens;
    size_t m_current;
    bool m_bHadError;
};

} // namespace nex


#endif