#ifndef NEX_STMT_HPP_
#define NEX_STMT_HPP_

#include "nex_token.hpp"
#include "nex_expr.hpp"
#include <memory>
#include <vector>

namespace nex::ast::stmt {
struct Block;
struct Class;
struct Expression;
struct Function;
struct If;
struct Print;
struct Return;
struct Let;
struct While;

class Visitor {
public:
    virtual std::any visitBlockStmt(Block* stmt) = 0;
    virtual std::any visitClassStmt(Class* stmt) = 0;
    virtual std::any visitExpressionStmt(Expression* stmt) = 0;
    virtual std::any visitFunctionStmt(Function* stmt) = 0;
    virtual std::any visitIfStmt(If* stmt) = 0;
    virtual std::any visitPrintStmt(Print* stmt) = 0;
    virtual std::any visitReturnStmt(Return* stmt) = 0;
    virtual std::any visitLetStmt(Let* stmt) = 0;
    virtual std::any visitWhileStmt(While* stmt) = 0;
};

struct Stmt {
    virtual std::any accept(Visitor* visitor) = 0;
};

struct Block : public Stmt {
    Block(std::vector<std::shared_ptr<Stmt>> statements) :
        m_statements(statements)
    {}

    virtual ~Block() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitBlockStmt(this);
    }

    const std::vector<std::shared_ptr<Stmt>> m_statements;
};

inline std::shared_ptr<Stmt> make_block(std::vector<std::shared_ptr<Stmt>> statements) {
    return std::make_shared<Block>(statements);
}

struct Class : public Stmt {
    Class(Token name, std::shared_ptr<expr::Variable> superclass, std::vector<std::shared_ptr<Function>> methods, std::vector<std::shared_ptr<Let>> fields) :
        m_name(name),
        m_superclass(superclass),
        m_methods(methods),
        m_fields(fields)
    {}

    virtual ~Class() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitClassStmt(this);
    }

    const Token m_name;
    const std::shared_ptr<expr::Variable> m_superclass;
    const std::vector<std::shared_ptr<Function>> m_methods;
    const std::vector<std::shared_ptr<Let>> m_fields;
};

inline std::shared_ptr<Stmt> make_class(Token name, std::shared_ptr<expr::Variable> superclass, std::vector<std::shared_ptr<Function>> methods, std::vector<std::shared_ptr<Let>> fields) {
    return std::make_shared<Class>(name, superclass, methods, fields);
}

struct Expression : public Stmt {
    Expression(std::shared_ptr<expr::Expr> e) :
        m_e(e)
    {}

    virtual ~Expression() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitExpressionStmt(this);
    }

    const std::shared_ptr<expr::Expr> m_e;
};

inline std::shared_ptr<Stmt> make_expression(std::shared_ptr<expr::Expr> e) {
    return std::make_shared<Expression>(e);
}

struct Function : public Stmt {
    Function(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt>> body) :
        m_name(name),
        m_params(params),
        m_body(body)
    {}

    virtual ~Function() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitFunctionStmt(this);
    }

    const Token m_name;
    const std::vector<Token> m_params;
    const std::vector<std::shared_ptr<Stmt>> m_body;
};

inline std::shared_ptr<Stmt> make_function(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt>> body) {
    return std::make_shared<Function>(name, params, body);
}

struct If : public Stmt {
    If(std::shared_ptr<expr::Expr> cond, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch) :
        m_cond(cond),
        m_thenBranch(thenBranch),
        m_elseBranch(elseBranch)
    {}

    virtual ~If() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitIfStmt(this);
    }

    const std::shared_ptr<expr::Expr> m_cond;
    const std::shared_ptr<Stmt> m_thenBranch;
    const std::shared_ptr<Stmt> m_elseBranch;
};

inline std::shared_ptr<Stmt> make_if(std::shared_ptr<expr::Expr> cond, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch) {
    return std::make_shared<If>(cond, thenBranch, elseBranch);
}

struct Print : public Stmt {
    Print(std::shared_ptr<expr::Expr> e) :
        m_e(e)
    {}

    virtual ~Print() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitPrintStmt(this);
    }

    const std::shared_ptr<expr::Expr> m_e;
};

inline std::shared_ptr<Stmt> make_print(std::shared_ptr<expr::Expr> e) {
    return std::make_shared<Print>(e);
}

struct Return : public Stmt {
    Return(Token keyword, std::shared_ptr<expr::Expr> value) :
        m_keyword(keyword),
        m_value(value)
    {}

    virtual ~Return() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitReturnStmt(this);
    }

    const Token m_keyword;
    const std::shared_ptr<expr::Expr> m_value;
};

inline std::shared_ptr<Stmt> make_return(Token keyword, std::shared_ptr<expr::Expr> value) {
    return std::make_shared<Return>(keyword, value);
}

struct Let : public Stmt {
    Let(Token name, std::shared_ptr<expr::Expr> init) :
        m_name(name),
        m_init(init)
    {}

    virtual ~Let() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitLetStmt(this);
    }

    const Token m_name;
    const std::shared_ptr<expr::Expr> m_init;
};

inline std::shared_ptr<Stmt> make_let(Token name, std::shared_ptr<expr::Expr> init) {
    return std::make_shared<Let>(name, init);
}

struct While : public Stmt {
    While(std::shared_ptr<expr::Expr> cond, std::shared_ptr<Stmt> body) :
        m_cond(cond),
        m_body(body)
    {}

    virtual ~While() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitWhileStmt(this);
    }

    const std::shared_ptr<expr::Expr> m_cond;
    const std::shared_ptr<Stmt> m_body;
};

inline std::shared_ptr<Stmt> make_while(std::shared_ptr<expr::Expr> cond, std::shared_ptr<Stmt> body) {
    return std::make_shared<While>(cond, body);
}

}
#endif
