#ifndef NEX_INTERPRETER_HPP
#define NEX_INTERPRETER_HPP

#include "nex_expr.hpp"
#include "nex_stmt.hpp"
#include "nex_environment.hpp"
#include <iostream>
#include <locale>
#include <codecvt>

namespace nex {

using namespace nex::ast;

class Interpreter final : public expr::Visitor, public stmt::Visitor
{
public:
    Interpreter();

    ~Interpreter() = default;

    void interpret(std::vector<std::shared_ptr<stmt::Stmt>> stmts);

    inline bool error() const { return m_bHadRuntimeError; }

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
    std::any visitInputExpr(expr::Input* stmt) override;

    std::any visitBlockStmt(stmt::Block* stmt) override;
    std::any visitClassStmt(stmt::Class* stmt) override;
    std::any visitFunctionStmt(stmt::Function* stmt) override;
    std::any visitIfStmt(stmt::If* stmt) override;
    std::any visitExpressionStmt(stmt::Expression* stmt) override;
    std::any visitPrintStmt(stmt::Print* stmt) override;
    std::any visitLetStmt(stmt::Let* stmt) override;
    std::any visitWhileStmt(stmt::While* stmt) override;
    std::any visitReturnStmt(stmt::Return* stmt) override;

    inline std::shared_ptr<Environment> getGlobalEnv() const
    {
        return m_pGlobals;
    }

    inline std::shared_ptr<Environment> getEnv() const
    {
        return m_pEnv;
    }

    void execute(std::shared_ptr<stmt::Stmt> s);
    void executeBlock(std::vector<std::shared_ptr<stmt::Stmt>> statements,
                      std::shared_ptr<Environment> env);

    void resolve(expr::Expr* expr, size_t idx);

private:
    std::any evaluate(std::shared_ptr<expr::Expr> e);
    bool isTruthy(std::any e);
    bool isEqual(std::any right, std::any left);
    std::wstring stringify(const std::any& value);
    void checkNumberOperand(const Token& op, const std::any& operand);
    void checkNumberOperands(const Token& op,
                             const std::any& left,
                             const std::any& right);
    std::any lookUpVariable(Token const& name, expr::Expr* expr);

private:
    bool m_bHadRuntimeError;
    std::shared_ptr<Environment> m_pEnv;
    std::shared_ptr<Environment> m_pGlobals;
    std::map<expr::Expr*, int> m_locals;
};


}


#endif
