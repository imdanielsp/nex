#include "nex_resolver.hpp"
#include "nex_diag.hpp"

namespace nex {

Resolver::Resolver(std::shared_ptr<Interpreter> pInterp)
    : m_bHadError(false)
    , m_pInterp(pInterp)
    , m_scopes()
    , m_currentFunctionType(FNONE)
    , m_currentClassType(CNONE)
{}

std::any Resolver::visitBlockStmt(stmt::Block* stmt)
{
    beginScope();
    resolve(stmt->m_statements);
    endScope();
    return nullptr;
}

std::any Resolver::visitClassStmt(stmt::Class* stmt)
{
    ClassType enclosingClass = m_currentClassType;
    m_currentClassType = CLASS;

    declare(stmt->m_name);
    define(stmt->m_name);

    if (stmt->m_superclass &&
        stmt->m_superclass->m_name.m_lexeme == stmt->m_name.m_lexeme) {
        ::nex::error(stmt->m_superclass->m_name.m_line, L"A class cannot inherit from itself");
        m_bHadError = true;
    }

    if (stmt->m_superclass) {
        m_currentClassType = SUBCLASS;
        resolve(stmt->m_superclass);
    }

    if (stmt->m_superclass) {
        beginScope();
        (*m_scopes.back())[L"super"] = true;
    }

    beginScope();
    (*m_scopes.back())[L"this"] = true;

    for (auto field : stmt->m_fields) {
        if (field->m_init != nullptr) {
            resolve(field->m_init);
        }
    }

    for (auto method : stmt->m_methods) {
        auto funcType = METHOD;
        if (method->m_name.m_lexeme == L"init") {
            funcType = INITIALIZER;
        }
        resolveFunction(method.get(), funcType);
    }

    endScope();

    if (stmt->m_superclass) {
        endScope();
    }

    m_currentClassType = enclosingClass;
    return nullptr;
}

std::any Resolver::visitExpressionStmt(stmt::Expression* stmt)
{
    resolve(stmt->m_e);
    return nullptr;
}

std::any Resolver::visitFunctionStmt(stmt::Function* stmt)
{
    declare(stmt->m_name);
    define(stmt->m_name);

    resolveFunction(stmt, FUNCTION);
    return nullptr;
}

std::any Resolver::visitIfStmt(stmt::If* stmt)
{
    resolve(stmt->m_cond);
    resolve(stmt->m_thenBranch);
    if (stmt->m_elseBranch) {
        resolve(stmt->m_elseBranch);
    }
    return nullptr;
}

std::any Resolver::visitPrintStmt(stmt::Print* stmt)
{
    resolve(stmt->m_e);
    return nullptr;
}

std::any Resolver::visitReturnStmt(stmt::Return* stmt)
{
    if (m_currentFunctionType == FNONE) {
        ::nex::error(stmt->m_keyword.m_line, L"Illegal return statement");
        m_bHadError = true;
    }

    if (stmt->m_value) {
        if (m_currentFunctionType == INITIALIZER) {
            ::nex::error(stmt->m_keyword.m_line, L"Cannot return a value from an initializer");
            m_bHadError = true;
        }
        resolve(stmt->m_value);
    }
    return nullptr;
}

std::any Resolver::visitLetStmt(stmt::Let* stmt)
{
    declare(stmt->m_name);
    if (stmt->m_init != nullptr) {
        resolve(stmt->m_init);
    }
    define(stmt->m_name);
    return nullptr;
}

std::any Resolver::visitWhileStmt(stmt::While* stmt)
{
    resolve(stmt->m_cond);
    resolve(stmt->m_body);
    return nullptr;
}

std::any Resolver::visitAssignExpr(expr::Assign* expr)
{
    resolve(expr->m_value);
    resolveLocal(expr, expr->m_name);
    return nullptr;
}

std::any Resolver::visitBinaryExpr(expr::Binary* expr)
{
    resolve(expr->m_left);
    resolve(expr->m_right);
    return nullptr;
}

std::any Resolver::visitCallExpr(expr::Call* expr)
{
    resolve(expr->m_callee);

    for (auto arg : expr->m_arguments) {
        resolve(arg);
    }

    return nullptr;
}

std::any Resolver::visitGetExpr(expr::Get* expr)
{
    resolve(expr->m_object);
    return nullptr;
}

std::any Resolver::visitSetExpr(expr::Set* expr)
{
    resolve(expr->m_value);
    resolve(expr->m_object);
    return nullptr;
}

std::any Resolver::visitSuperExpr(expr::Super* expr)
{
    if (m_currentClassType == CNONE) {
        ::nex::error(expr->m_keyword.m_line, L"Cannot use 'super' outside of a class");
        m_bHadError = true;
    }
    else if (m_currentClassType != SUBCLASS) {
        ::nex::error(expr->m_keyword.m_line, L"Cannot use 'super' in a class with not superclass.");
        m_bHadError = true;
    }

    resolveLocal(expr, expr->m_keyword);
    return nullptr;
};

std::any Resolver::visitThisExpr(expr::This* expr)
{
    if (m_currentClassType == CNONE) {
        ::nex::error(expr->m_keyword.m_line, L"Cannot use 'this' outside of a class");
        m_bHadError = true;
        return nullptr;
    }

    resolveLocal(expr, expr->m_keyword);
    return nullptr;
}

std::any Resolver::visitGroupingExpr(expr::Grouping* expr)
{
    resolve(expr->m_expression);
    return nullptr;
}

std::any Resolver::visitLiteralExpr(expr::Literal* expr)
{
    (void) expr;
    return nullptr;
}

std::any Resolver::visitLogicalExpr(expr::Logical* expr)
{
    resolve(expr->m_left);
    resolve(expr->m_right);
    return nullptr;
}

std::any Resolver::visitUnaryExpr(expr::Unary* expr)
{
    resolve(expr->m_right);
    return nullptr;
}

std::any Resolver::visitCommaExpr(expr::Comma* expr)
{
    (void) expr;
    return nullptr;
}

std::any Resolver::visitVariableExpr(expr::Variable* expr)
{
    if (!m_scopes.empty() &&
        m_scopes.back()->count(expr->m_name.m_lexeme) != 0 &&
        m_scopes.back()->at(expr->m_name.m_lexeme) == false) {
        ::nex::error(expr->m_name.m_line, L"Cannot read local variable in its own initializer");
        m_bHadError = true;
    }

    resolveLocal(expr, expr->m_name);
    return nullptr;
}

std::any Resolver::visitInputExpr(expr::Input* expr)
{
    (void) expr;
    return nullptr;
}

void Resolver::resolve(const std::vector<std::shared_ptr<stmt::Stmt>> statements)
{
    for (auto pStmt : statements) {
        resolve(pStmt);
    }
}

void Resolver::resolve(std::shared_ptr<stmt::Stmt> statement)
{
    statement->accept(this);
}

void Resolver::beginScope()
{
    m_scopes.push_back(std::make_shared<std::unordered_map<std::wstring, bool>>());
}

void Resolver::endScope()
{
    m_scopes.pop_back();
}

void Resolver::resolve(std::shared_ptr<expr::Expr> expr)
{
    expr->accept(this);
}

void Resolver::define(Token const& name)
{
    if (m_scopes.empty()) {
        return;
    }

    (*m_scopes.back())[name.m_lexeme] = true;
}

void Resolver::declare(Token const& name)
{
    if (m_scopes.empty()) {
        return;
    }

    auto scope = m_scopes.back();
    if (scope->count(name.m_lexeme)) {
        ::nex::error(name.m_line, L"Identifier '" + name.m_lexeme + L"' has already been declared");
        m_bHadError = true;
    }
    (*scope)[name.m_lexeme] = false;
}

void Resolver::resolveLocal(expr::Expr* expr, Token const& name)
{
    for (int idx = m_scopes.size() - 1; idx >= 0; idx--) {
        if (m_scopes[idx]->count(name.m_lexeme)) {
            m_pInterp->resolve(expr, m_scopes.size() - 1 - idx);
            return;
        }
    }
}

void Resolver::resolveFunction(stmt::Function* func, FunctionType funcType)
{
    auto enclosingFunction = m_currentFunctionType;
    m_currentFunctionType = funcType;

    beginScope();
    for (auto param : func->m_params) {
        declare(param);
        define(param);
    }
    resolve(func->m_body);
    endScope();

    m_currentFunctionType = enclosingFunction;
}

}