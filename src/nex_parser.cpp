#include "nex_parser.hpp"
#include "nex_diag.hpp"

namespace nex {

std::vector<StmtPointer> Parser::parse()
{
    std::vector<StmtPointer> stmts;
    try {
        while (!isAtEnd()) {
            stmts.push_back(declaration());
        }
    } catch (const std::runtime_error e) {
        m_bHadError = false;
        return stmts;
    }
    return stmts;
}

StmtPointer Parser::declaration()
{
    try {
        if (match(CLASS)) {
            return classDeclaration();
        }

        if (match(FUNC)) {
            return function(L"function");
        }

        if (match(LET)) {
            return letDeclaration();
        }

        return statement();
    } catch (const ParserError& e) {
        synchronize();
        return nullptr;
    }
}

StmtPointer Parser::classDeclaration()
{
    auto name = consume(IDENTIFIER, L"Expect class name.");

    std::shared_ptr<Variable> superclass = nullptr;
    if (match(EXTENDS)) {
        consume(IDENTIFIER, L"Expect superclass name.");
        superclass = std::dynamic_pointer_cast<Variable>(make_variable(previous()));
    }

    consume(LEFT_BRACE, L"Expect '{' before class body");

    std::vector<std::shared_ptr<Function>> methods;
    std::vector<std::shared_ptr<Let>> fields;
    while (!check(RIGHT_BRACE) && !isAtEnd()) {
        if (match(LET)) {
            fields.push_back(std::dynamic_pointer_cast<Let>(letDeclaration()));
        }
        else if (match(FUNC)) {
            methods.push_back(std::dynamic_pointer_cast<Function>(function(L"method")));
        }
        else {
            throw error(peek(), L"Unexpected token '" + peek().m_lexeme + L"'.");
        }
    }

    consume(RIGHT_BRACE, L"Expect '}' after class body");
    return make_class(name, superclass, methods, fields);
}

StmtPointer Parser::function(const std::wstring& kind)
{
    auto name = consume(IDENTIFIER, L"Expect " + kind + L" name.");

    consume(LEFT_PAREN, L"Expect '(' after " + kind + L" name.");

    std::vector<Token> parameters;
    if (!check(RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                error(peek(), L"Cannot have more than 255 parameters");
            }

            parameters.push_back(consume(IDENTIFIER, L"Expect parameter name."));
        } while (match(COMMA));
    }
    consume(RIGHT_PAREN, L"Expect ')' after parameters");

    consume(LEFT_BRACE, L"Expect '}' before " + kind + L" body.");
    auto body = block();

    return make_function(name, parameters, body);
}

StmtPointer Parser::letDeclaration()
{
    Token name = consume(IDENTIFIER, L"Expect variable name");

    ExprPointer init = nullptr;
    if (match(EQUAL)) {
        init = expression();
    }

    consume(SEMICOLON, L"Expect ';' after variable declaration");
    return make_let(name, init);
}

StmtPointer Parser::statement()
{
    if (match(FOR)) {
        return forStatement();
    }

    if (match(IF)) {
        return ifStatement();
    }

    if (match(PRINT)) {
        return printStatement();
    }

    if (match(RET)) {
        return returnStatement();
    }

    if (match(WHILE)) {
        return whileStatement();
    }

    if (match(LEFT_BRACE)) {
        auto b = block();
        return make_block(b);
    }

    return expressionStatement();
}

StmtPointer Parser::forStatement()
{
    consume(LEFT_PAREN, L"Expect '(' after 'for'.");
    StmtPointer init;
    if (match(SEMICOLON)) {
        init = nullptr;
    } else if (match((LET))) {
        init = letDeclaration();
    } else {
        init = expressionStatement();
    }

    ExprPointer cond = nullptr;
    if (!check(SEMICOLON)) {
        cond = expression();
    }

    consume(SEMICOLON, L"Expect ';' after loop condition");

    ExprPointer increment = nullptr;
    if (!check(RIGHT_PAREN)) {
        increment = expression();
    }
    consume(RIGHT_PAREN, L"Expect ')' after for clauses");

    StmtPointer body = statement();

    // Extand the syntatic sugar of a for-loop to a while-loop:
    if(increment != nullptr) {
        body = make_block({
            body,
            make_expression(increment),
        });
    }

    if (cond == nullptr) {
        cond = make_literal(true);
    }
    body = make_while(cond, body);

    if (init != nullptr) {
        body = make_block({ init, body });
    }

    return body;
}

StmtPointer Parser::ifStatement()
{
    consume(LEFT_PAREN, L"Expect '(' after 'if'.");
    auto cond = expression();
    consume(RIGHT_PAREN, L"Expect ')' after if condition.");

    auto thenBranch = statement();

    StmtPointer elseBranch = nullptr;
    if (match(ELSE)) {
        elseBranch = statement();
    }

    return make_if(cond, thenBranch, elseBranch);
}

std::vector<StmtPointer> Parser::block()
{
    std::vector<StmtPointer> statements;

    while (!check(RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    consume(RIGHT_BRACE, L"Expect '}' after block.");
    return statements;
}

StmtPointer Parser::printStatement()
{
    consume(LEFT_PAREN, L"Expect '(' before function call");
    auto expr = expression();
    consume(RIGHT_PAREN, L"Expect ')' after function call");
    consume(SEMICOLON, L"Expect ';' after value");
    return make_print(expr);
}

StmtPointer Parser::returnStatement()
{
    auto keyword = previous();
    ExprPointer value = nullptr;
    if (!check(SEMICOLON)) {
        value = expression();
    }

    consume(SEMICOLON, L"Expect ';' after return value");
    return make_return(keyword, value);
}

StmtPointer Parser::whileStatement()
{
    consume(LEFT_PAREN, L"Expect '(' after 'while'.");
    auto cond = expression();
    consume(RIGHT_PAREN, L"Expect ')' after condition.");
    auto stmt = statement();

    return make_while(cond, stmt);
}

ExprPointer Parser::inputExpr()
{
    consume(LEFT_PAREN, L"Expect '(' before function call");
    consume(RIGHT_PAREN, L"Expect ')' after function call");
    return make_input(nullptr);
}

StmtPointer Parser::expressionStatement()
{
    auto expr = expression();
    consume(SEMICOLON, L"Expect ';' after expression.");
    return make_expression(expr);
}

ExprPointer Parser::assignment()
{
    auto expr = orExpr();

    if (match(EQUAL)) {
        auto equals = previous();
        auto value = assignment();

        if (auto pVar = std::dynamic_pointer_cast<Variable>(expr)) {
            auto name = pVar->m_name;
            return make_assign(name, value);
        }
        else if (auto pGet = std::dynamic_pointer_cast<Get>(expr)) {
            return make_set(pGet->m_object, pGet->m_name, value);
        }

        error(equals, L"Invalid assignment target.");
    }

    return expr;
}

ExprPointer Parser::orExpr()
{
    auto expr = andExpr();

    while (match(OR)) {
        auto op = previous();
        auto right = andExpr();
        expr = make_logical(expr, op, right);
    }

    return expr;
}

ExprPointer Parser::andExpr()
{
    auto expr = equality();

    while (match(AND)) {
        auto op = previous();
        auto right = equality();
        expr = make_logical(expr, op, right);
    }

    return expr;
}

ExprPointer Parser::expression()
{
    return assignment();
}

ExprPointer Parser::equality()
{
    auto expr = comparison();

    while (match(BANG_EQUAL, EQUAL_EQUAL)) {
        auto op = previous();
        auto right = comparison();
        expr = make_binary(expr, op, right);
    }

    return expr;
}

ExprPointer Parser::comparison()
{
    auto expr = addition();

    while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
        auto op = previous();
        auto right = addition();
        expr = make_binary(expr, op, right);
    }

    return expr;
}

ExprPointer Parser::addition()
{
    auto expr = multiplication();

    while (match(MINUS, PLUS)) {
        auto op = previous();
        auto right = multiplication();
        expr = make_binary(expr, op, right);
    }

    return expr;
}

ExprPointer Parser::multiplication()
{
    auto expr = unary();

    while (match(SLASH, STAR)) {
        auto op = previous();
        auto right = unary();
        expr = make_binary(expr, op, right);
    }

    return expr;
}

ExprPointer Parser::unary()
{
    if (match(BANG, MINUS)) {
        auto op = previous();
        auto right = unary();
        return make_unary(op, right);
    }

    return call();
}

ExprPointer Parser::call()
{
    auto expr = primary();

    while (true) {
        if (match(LEFT_PAREN)) {
            expr = finishCall(expr);
        } else if (match(DOT)) {
            auto name = consume(IDENTIFIER, L"Expect property name after '.'");
            expr = make_get(expr, name);
        } else {
            break;
        }
    }

    return expr;
}

ExprPointer Parser::finishCall(ExprPointer e)
{
    std::vector<ExprPointer> arguments;
    if (!check(RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255) {
                error(peek(), L"Cannot have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match(COMMA));
    }

    Token paren = consume(RIGHT_PAREN, L"Expect ')' after arguments.");

    return make_call(e, paren, arguments);
}

ExprPointer Parser::primary()
{
    if (match(FALSE)) {
        return make_literal(false);
    }

    if (match(TRUE)) {
        return make_literal(true);
    }

    if (match(NIL)) {
        return make_literal(nullptr);
    }

    if (match(NUMBER, STRING)) {
        return make_literal(previous().m_literal);
    }

    if (match(LEFT_PAREN)) {
        auto expr = expression();

        if (match(COMMA)) {
            std::vector<ExprPointer> es;
            es.push_back(expr);
            auto last = expression();
            while (match(COMMA)) {
                auto e = expression();
                es.push_back(last);
                last = e;

                if (check(RIGHT_PAREN)) {
                    break;
                }
            }
            consume(RIGHT_PAREN, L"Expect ')' after expression");
            return make_comma(es, last);
        }
        else {
            consume(RIGHT_PAREN, L"Expect ')' after expression");
            return make_grouping(expr);
        }
    }

    if (match(INPUT)) {
        return inputExpr();
    }

    if (match(SUPER)) {
        auto keyword = previous();
        consume(DOT, L"Expect '.' after 'super'.");
        auto method = consume(IDENTIFIER, L"Expect superclass method name");
        return make_super(keyword, method);
    }

    if (match(THIS)) {
        return make_this(previous());
    }

    if (match(IDENTIFIER)) {
        return make_variable(previous());
    }

    throw error(peek(), L"Expect expression");
}

ExprPointer Parser::comma(ExprPointer e)
{
    (void) e;
    return nullptr;
}

Token Parser::consume(TokenType type, std::wstring msg)
{
    if (check(type)) {
        return advance();
    }

    throw error(peek(), msg);
}

void Parser::reportError(Token token, std::wstring msg)
{
    if (token.m_type == END_OF_FILE) {
        ::nex::report(token.m_line, L" at end", msg);
    }
    else {
        ::nex::report(token.m_line, L" at '" + token.m_lexeme + L"'", msg);
    }
}
ParserError Parser::error(Token token, std::wstring msg)
{
    m_bHadError = true;
    ::nex::report(token.m_line, L"", msg);
    return ParserError(L"");
}

bool Parser::check(TokenType type)
{
    if (isAtEnd()) {
        return false;
    }

    return peek().m_type == type;
}

Token Parser::advance()
{
    if (!isAtEnd()) {
        m_current++;
    }
    return previous();
}

void Parser::synchronize()
{
    advance();

    while (!isAtEnd()) {
        if (previous().m_type == SEMICOLON) return;

        switch (peek().m_type) {
            case CLASS:
            case FUNC:
            case CONST:
            case LET:
            case FOR:
            case IF:
            case WHILE:
            case PRINT:
            case RET:
                return;
        }

        advance();
    }
}

}
