#ifndef NEX_EXPR_HPP_
#define NEX_EXPR_HPP_

#include "nex_token.hpp"
#include <memory>
#include <vector>

namespace nex::ast::expr {
struct Assign;
struct Binary;
struct Call;
struct Get;
struct Set;
struct Super;
struct This;
struct Grouping;
struct Literal;
struct Logical;
struct Unary;
struct Comma;
struct Variable;
struct Input;

class Visitor {
public:
    virtual std::any visitAssignExpr(Assign* expr) = 0;
    virtual std::any visitBinaryExpr(Binary* expr) = 0;
    virtual std::any visitCallExpr(Call* expr) = 0;
    virtual std::any visitGetExpr(Get* expr) = 0;
    virtual std::any visitSetExpr(Set* expr) = 0;
    virtual std::any visitSuperExpr(Super* expr) = 0;
    virtual std::any visitThisExpr(This* expr) = 0;
    virtual std::any visitGroupingExpr(Grouping* expr) = 0;
    virtual std::any visitLiteralExpr(Literal* expr) = 0;
    virtual std::any visitLogicalExpr(Logical* expr) = 0;
    virtual std::any visitUnaryExpr(Unary* expr) = 0;
    virtual std::any visitCommaExpr(Comma* expr) = 0;
    virtual std::any visitVariableExpr(Variable* expr) = 0;
    virtual std::any visitInputExpr(Input* expr) = 0;
};

struct Expr {
    virtual std::any accept(Visitor* visitor) = 0;
};

struct Assign : public Expr {
    Assign(Token name, std::shared_ptr<Expr> value) :
        m_name(name),
        m_value(value)
    {}

    virtual ~Assign() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitAssignExpr(this);
    }

    const Token m_name;
    const std::shared_ptr<Expr> m_value;
};

inline std::shared_ptr<Expr> make_assign(Token name, std::shared_ptr<Expr> value) {
    return std::make_shared<Assign>(name, value);
}

struct Binary : public Expr {
    Binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right) :
        m_left(left),
        m_op(op),
        m_right(right)
    {}

    virtual ~Binary() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitBinaryExpr(this);
    }

    const std::shared_ptr<Expr> m_left;
    const Token m_op;
    const std::shared_ptr<Expr> m_right;
};

inline std::shared_ptr<Expr> make_binary(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right) {
    return std::make_shared<Binary>(left, op, right);
}

struct Call : public Expr {
    Call(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments) :
        m_callee(callee),
        m_paren(paren),
        m_arguments(arguments)
    {}

    virtual ~Call() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitCallExpr(this);
    }

    const std::shared_ptr<Expr> m_callee;
    const Token m_paren;
    const std::vector<std::shared_ptr<Expr>> m_arguments;
};

inline std::shared_ptr<Expr> make_call(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments) {
    return std::make_shared<Call>(callee, paren, arguments);
}

struct Get : public Expr {
    Get(std::shared_ptr<Expr> object, Token name) :
        m_object(object),
        m_name(name)
    {}

    virtual ~Get() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitGetExpr(this);
    }

    const std::shared_ptr<Expr> m_object;
    const Token m_name;
};

inline std::shared_ptr<Expr> make_get(std::shared_ptr<Expr> object, Token name) {
    return std::make_shared<Get>(object, name);
}

struct Set : public Expr {
    Set(std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value) :
        m_object(object),
        m_name(name),
        m_value(value)
    {}

    virtual ~Set() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitSetExpr(this);
    }

    const std::shared_ptr<Expr> m_object;
    const Token m_name;
    const std::shared_ptr<Expr> m_value;
};

inline std::shared_ptr<Expr> make_set(std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value) {
    return std::make_shared<Set>(object, name, value);
}

struct Super : public Expr {
    Super(Token keyword, Token method) :
        m_keyword(keyword),
        m_method(method)
    {}

    virtual ~Super() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitSuperExpr(this);
    }

    const Token m_keyword;
    const Token m_method;
};

inline std::shared_ptr<Expr> make_super(Token keyword, Token method) {
    return std::make_shared<Super>(keyword, method);
}

struct This : public Expr {
    This(Token keyword) :
        m_keyword(keyword)
    {}

    virtual ~This() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitThisExpr(this);
    }

    const Token m_keyword;
};

inline std::shared_ptr<Expr> make_this(Token keyword) {
    return std::make_shared<This>(keyword);
}

struct Grouping : public Expr {
    Grouping(std::shared_ptr<Expr> expression) :
        m_expression(expression)
    {}

    virtual ~Grouping() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitGroupingExpr(this);
    }

    const std::shared_ptr<Expr> m_expression;
};

inline std::shared_ptr<Expr> make_grouping(std::shared_ptr<Expr> expression) {
    return std::make_shared<Grouping>(expression);
}

struct Literal : public Expr {
    Literal(std::any value) :
        m_value(value)
    {}

    virtual ~Literal() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitLiteralExpr(this);
    }

    const std::any m_value;
};

inline std::shared_ptr<Expr> make_literal(std::any value) {
    return std::make_shared<Literal>(value);
}

struct Logical : public Expr {
    Logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right) :
        m_left(left),
        m_op(op),
        m_right(right)
    {}

    virtual ~Logical() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitLogicalExpr(this);
    }

    const std::shared_ptr<Expr> m_left;
    const Token m_op;
    const std::shared_ptr<Expr> m_right;
};

inline std::shared_ptr<Expr> make_logical(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right) {
    return std::make_shared<Logical>(left, op, right);
}

struct Unary : public Expr {
    Unary(Token op, std::shared_ptr<Expr> right) :
        m_op(op),
        m_right(right)
    {}

    virtual ~Unary() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitUnaryExpr(this);
    }

    const Token m_op;
    const std::shared_ptr<Expr> m_right;
};

inline std::shared_ptr<Expr> make_unary(Token op, std::shared_ptr<Expr> right) {
    return std::make_shared<Unary>(op, right);
}

struct Comma : public Expr {
    Comma(std::vector<std::shared_ptr<Expr>> exprs, std::shared_ptr<Expr> last) :
        m_exprs(exprs),
        m_last(last)
    {}

    virtual ~Comma() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitCommaExpr(this);
    }

    const std::vector<std::shared_ptr<Expr>> m_exprs;
    const std::shared_ptr<Expr> m_last;
};

inline std::shared_ptr<Expr> make_comma(std::vector<std::shared_ptr<Expr>> exprs, std::shared_ptr<Expr> last) {
    return std::make_shared<Comma>(exprs, last);
}

struct Variable : public Expr {
    Variable(Token name) :
        m_name(name)
    {}

    virtual ~Variable() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitVariableExpr(this);
    }

    const Token m_name;
};

inline std::shared_ptr<Expr> make_variable(Token name) {
    return std::make_shared<Variable>(name);
}

struct Input : public Expr {
    Input(void* e) :
        m_e(e)
    {}

    virtual ~Input() = default;

    std::any accept(Visitor* visitor) {
        return visitor->visitInputExpr(this);
    }

    const void* m_e;
};

inline std::shared_ptr<Expr> make_input(void* e) {
    return std::make_shared<Input>(e);
}

}
#endif
