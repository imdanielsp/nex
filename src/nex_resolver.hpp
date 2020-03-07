#ifndef NEX_RESOLVER_HPP
#define NEX_RESOLVER_HPP

#include "nex_expr.hpp"
#include "nex_interpreter.hpp"
#include "nex_stmt.hpp"
#include "nex_token.hpp"

#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

namespace nex
{

class Resolver final : public stmt::Visitor, public expr::Visitor
{
public:
    Resolver(std::shared_ptr<Interpreter> pInterp);
    ~Resolver() = default;

    std::any visitBlockStmt(stmt::Block* stmt) override;
    std::any visitClassStmt(stmt::Class* stmt) override;
    std::any visitExpressionStmt(stmt::Expression* stmt) override;
    std::any visitFunctionStmt(stmt::Function* stmt) override;
    std::any visitIfStmt(stmt::If* stmt) override;
    std::any visitPrintStmt(stmt::Print* stmt) override;
    std::any visitReturnStmt(stmt::Return* stmt) override;
    std::any visitLetStmt(stmt::Let* stmt) override;
    std::any visitWhileStmt(stmt::While* stmt) override;

    std::any visitAssignExpr(expr::Assign* expr) override;
    std::any visitBinaryExpr(expr::Binary* expr) override;
    std::any visitCallExpr(expr::Call* expr) override;
    std::any visitGetExpr(expr::Get* expr) override;
    std::any visitSetExpr(expr::Set* expr) override;
    std::any visitSuperExpr(expr::Super* expr) override;
    std::any visitThisExpr(expr::This* expr) override;
    std::any visitGroupingExpr(expr::Grouping* expr) override;
    std::any visitLiteralExpr(expr::Literal* expr) override;
    std::any visitLogicalExpr(expr::Logical* expr) override;
    std::any visitUnaryExpr(expr::Unary* expr) override;
    std::any visitCommaExpr(expr::Comma* expr) override;
    std::any visitVariableExpr(expr::Variable* expr) override;
    std::any visitInputExpr(expr::Input* expr) override;

public:
    enum FunctionType {
        FNONE,
        FUNCTION,
        INITIALIZER,
        METHOD,
    };

    enum ClassType {
        CNONE,
        CLASS,
        SUBCLASS,
    };

    void resolve(const std::vector<std::shared_ptr<stmt::Stmt>> statements);
    void resolve(std::shared_ptr<stmt::Stmt> statement);
    void beginScope();
    void endScope();
    void resolve(std::shared_ptr<expr::Expr> expr);
    void declare(Token const& name);
    void define(Token const& name);
    void resolveLocal(expr::Expr* expr, Token const& name);
    void resolveFunction(stmt::Function* func, FunctionType funcType);
    inline bool error() const { return m_bHadError; }
private:
    bool m_bHadError;
    std::shared_ptr<Interpreter> m_pInterp;
    std::deque<std::shared_ptr<std::unordered_map<std::wstring, bool>>> m_scopes;
    FunctionType m_currentFunctionType;
    ClassType m_currentClassType;
};

} // namespace nex


#endif
