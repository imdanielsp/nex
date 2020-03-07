#include "nex_interpreter.hpp"
#include "nex_token.hpp"
#include "nex_diag.hpp"
#include "nex_callable.hpp"
#include "nex_runtime.hpp"
#include "nex_function.hpp"
#include "nex_class.hpp"
#include "nex_instance.hpp"
#include "nex_runtime_error.hpp"
#include "nex_return.hpp"

namespace nex {

using namespace nex::runtime;

Interpreter::Interpreter()
    : m_bHadRuntimeError(false)
    , m_pEnv(std::make_shared<Environment>(L"local"))
    , m_pGlobals(std::make_shared<Environment>(L"global"))
    , m_locals()
{
    // Insert native functions to the global environment
#define EMIT_NATIVE_FN(id, symbol)      \
    m_pGlobals->define(id, std::make_any<std::shared_ptr<NexCallable>>(std::make_shared<SystemClock>()));
    NATIVE_FN_LIST
#undef EMIT_NATIVE_FN

    // Copy the global environment into the interpreter's current stack environment
    m_pEnv->copy(m_pGlobals);
    m_pEnv->dump();
}

void Interpreter::interpret(std::vector<std::shared_ptr<stmt::Stmt>> stmts)
{
    try {
        for (auto s : stmts) {
            execute(s);
        }
    } catch (const NexRunTimeError& e) {
        m_bHadRuntimeError = true;
        runtimeError(e);
    }
}

void Interpreter::execute(std::shared_ptr<stmt::Stmt> s)
{
    s->accept(this);
}


std::any Interpreter::visitAssignExpr(expr::Assign* expr)
{
    auto value = evaluate(expr->m_value);
    if (m_locals.count(expr)) {
        auto distance = m_locals[expr];
        m_pEnv->assignAt(distance, expr->m_name, value);
    }
    return value;
}

std::any Interpreter::visitLiteralExpr(expr::Literal* expr)
{
    return expr->m_value;
}

std::any Interpreter::visitLogicalExpr(expr::Logical* expr)
{
    auto left = evaluate(expr->m_left);
    if (expr->m_op.m_type == OR) {
        if (isTruthy(left)) {
            return left;
        }
    }
    else {
        if (!isTruthy(left)) {
            return left;
        }
    }

    return evaluate(expr->m_right);
}

std::any Interpreter::visitGroupingExpr(expr::Grouping* expr)
{
    return evaluate(expr->m_expression);
}

std::any Interpreter::visitUnaryExpr(expr::Unary* expr)
{
    auto right = evaluate(expr->m_right);
    switch (expr->m_op.m_type) {
    case BANG:
        return !isTruthy(right);
    case MINUS:
        return -std::any_cast<double>(right);
    default:
        break;
    }

    return nullptr;
}

std::any Interpreter::visitBinaryExpr(expr::Binary* expr)
{
    auto left = evaluate(expr->m_left);
    auto right = evaluate(expr->m_right);

    switch (expr->m_op.m_type) {
    case GREATER:
        checkNumberOperands(expr->m_op, left, right);
        return std::any_cast<double>(left) > std::any_cast<double>(right);
    case GREATER_EQUAL:
        checkNumberOperands(expr->m_op, left, right);
        return std::any_cast<double>(left) >= std::any_cast<double>(right);
    case LESS:
        checkNumberOperands(expr->m_op, left, right);
        return std::any_cast<double>(left) < std::any_cast<double>(right);
    case LESS_EQUAL:
        checkNumberOperands(expr->m_op, left, right);
        return std::any_cast<double>(left) <= std::any_cast<double>(right);
    case MINUS:
        checkNumberOperands(expr->m_op, left, right);
        return std::any_cast<double>(left) - std::any_cast<double>(right);
    case SLASH:
    {
        checkNumberOperands(expr->m_op, left, right);
        auto rightVal = std::any_cast<double>(right);
        if (rightVal == 0) {
            throw NexRunTimeError(expr->m_op, L"Division by zero");
        }
        return std::any_cast<double>(left) / std::any_cast<double>(right);
    }
    case STAR:
        checkNumberOperands(expr->m_op, left, right);
        return std::any_cast<double>(left) * std::any_cast<double>(right);
    case PLUS:
    {
        if (auto pLeft = std::any_cast<double>(&left))
        if (auto pRight = std::any_cast<double>(&right)) {
            return *pLeft + *pRight;
        }

        if (auto pLeft = std::any_cast<std::wstring>(&left))
        if (auto pRight = std::any_cast<std::wstring>(&right)) {
            return *pLeft + *pRight;
        }

        throw NexRunTimeError(expr->m_op, L"Operans must be two numbers or two strings");
    }
    case BANG_EQUAL: return !isEqual(left, right);
    case EQUAL_EQUAL: return isEqual(left, right);
    default:
        break;
    }

    return nullptr;
}

std::any Interpreter::visitCallExpr(expr::Call* expr)
{
    std::any calle = evaluate(expr->m_callee);
    std::vector<std::any> arguments;
    for (auto arg : expr->m_arguments) {
        arguments.push_back(evaluate(arg));
    }

    if (std::any_cast<std::shared_ptr<NexCallable>>(&calle) == nullptr) {
        throw NexRunTimeError(expr->m_paren, L"Can only call functions and classes");
    }

    auto callable = std::any_cast<std::shared_ptr<NexCallable>>(calle);

    if (arguments.size() != callable->arity()) {
        throw NexRunTimeError(expr->m_paren,
            L"'" + callable->name() +
            L"' expected " + std::to_wstring(callable->arity()) +
            L" arguments but got " +
            std::to_wstring(arguments.size()) + L".");
    }

    return callable->call(this, arguments);
}

std::any Interpreter::visitGetExpr(expr::Get* expr)
{
    auto object = evaluate(expr->m_object);
    if (auto pObject = std::any_cast<std::shared_ptr<NexInstance>>(&object)) {
        return (*pObject)->get(expr->m_name);
    }

    throw NexRunTimeError(expr->m_name,
        L"Object has not property '" + expr->m_name.m_lexeme + L"'");
}

std::any Interpreter::visitSetExpr(expr::Set* expr)
{
    auto object = evaluate(expr->m_object);

    if (auto pInstance = std::any_cast<std::shared_ptr<NexInstance>>(&object)) {
        auto value = evaluate(expr->m_value);
        (*pInstance)->set(expr->m_name, value);
        return value;
    }

    throw NexRunTimeError(expr->m_name,
        L"Object has not property '" + expr->m_name.m_lexeme + L"'");
}

std::any Interpreter::visitSuperExpr(expr::Super* expr)
{
    auto distance = m_locals[expr];
    auto superclass = std::any_cast<std::shared_ptr<NexClass>>(m_pEnv->getAt(distance, L"super"));
    auto object = std::any_cast<std::shared_ptr<NexInstance>>(m_pEnv->getAt(distance - 1, L"this"));

    auto method = superclass->findMethod(expr->m_method.m_lexeme);

    if (!method) {
        throw NexRunTimeError(expr->m_method, L"Undefined property '" + expr->m_method.m_lexeme + L"'.");
    }

    return std::dynamic_pointer_cast<NexCallable>(method->bind(object));
}

std::any Interpreter::visitThisExpr(expr::This* expr)
{
    return lookUpVariable(expr->m_keyword, expr);
}

std::any Interpreter::visitCommaExpr(expr::Comma* expr)
{
    for (auto e : expr->m_exprs) {
        evaluate(e);
    }
    return evaluate(expr->m_last);
}

std::any Interpreter::visitVariableExpr(expr::Variable* expr)
{
    return lookUpVariable(expr->m_name, expr);
}

std::any Interpreter::visitInputExpr(expr::Input* expr)
{
    (void) expr;
    std::wstring in;
    std::wcin >> in;
    return in;
}

std::any Interpreter::visitIfStmt(stmt::If* stmt)
{
    if (isTruthy(evaluate(stmt->m_cond))) {
        execute(stmt->m_thenBranch);
    }
    else if (stmt->m_elseBranch != nullptr) {
        execute(stmt->m_elseBranch);
    }

    return nullptr;
}

std::any Interpreter::visitBlockStmt(stmt::Block* stmt)
{
    auto newEnv = std::make_shared<Environment>(L"block");
    newEnv->m_pEnclosing = m_pEnv;

    executeBlock(stmt->m_statements, newEnv);
    return nullptr;
}

std::any Interpreter::visitClassStmt(stmt::Class* stmt)
{
    std::shared_ptr<NexClass> superclass = nullptr;
    if (stmt->m_superclass) {
        std::any evalSuper = evaluate(stmt->m_superclass);
        auto ppCallable = std::any_cast<std::shared_ptr<NexCallable>>(&evalSuper);

        if (ppCallable) {
            superclass = std::dynamic_pointer_cast<NexClass>(*ppCallable);
        }

        if (!ppCallable || !superclass) {
            throw NexRunTimeError(stmt->m_superclass->m_name, L"Superclass must be a class.");
        }
    }

    m_pEnv->define(stmt->m_name, nullptr);

    if (stmt->m_superclass) {
        auto superEnv = std::make_shared<Environment>(L"super");
        superEnv->copy(m_pEnv);
        m_pEnv = superEnv;

        Token super(SUPER, L"super", nullptr, 0);
        superEnv->define(super, superclass);
    }

    NexClass::Fields fields;
    for (auto field : stmt->m_fields) {
        fields[field->m_name.m_lexeme] = field;
    }

    NexClass::Methods methods;
    for (auto  method : stmt->m_methods) {
        auto bIsInitializer = method->m_name.m_lexeme == L"init";

        methods[method->m_name.m_lexeme] =
            std::make_shared<NexFunction>(*method, m_pEnv, bIsInitializer);
    }

    auto klass = std::dynamic_pointer_cast<NexCallable>(
        std::make_shared<NexClass>(stmt->m_name.m_lexeme, superclass, fields, methods));

    if (superclass) {
        m_pEnv = m_pEnv->m_pEnclosing;
    }

    m_pEnv->assign(stmt->m_name, klass);

    return nullptr;
}

void  Interpreter::executeBlock(std::vector<std::shared_ptr<stmt::Stmt>> statements,
                                std::shared_ptr<Environment> pEnv)
{
    auto previous = m_pEnv;
    try {
        m_pEnv = pEnv;
        for (auto stmt : statements) {
            execute(stmt);
        }
    } catch (const NexReturn& e) {
        m_pEnv = previous;
        throw e;
    } catch (const NexRunTimeError& e) {
        m_pEnv = previous;
        throw e;
    } catch (...) {
        std::cout << "INTERPRETER ERROR: unhandled exception" << std::endl;
    }

    m_pEnv = previous;
}

std::any Interpreter::visitExpressionStmt(stmt::Expression* stmt)
{
    evaluate(stmt->m_e);
    return nullptr;
}

std::any Interpreter::visitFunctionStmt(stmt::Function* stmt)
{
    std::shared_ptr<NexCallable> func =
        std::make_shared<NexFunction>(*stmt, m_pEnv, false);
    m_pEnv->define(stmt->m_name, func);
    return nullptr;
}

std::any Interpreter::visitPrintStmt(stmt::Print* stmt)
{
    auto value = evaluate(stmt->m_e);
    std::wcout << stringify(value) << std::endl;
    return nullptr;
}

std::any Interpreter::visitLetStmt(stmt::Let* stmt)
{
    std::any value = nullptr;
    if (stmt->m_init != nullptr) {
        value = evaluate(stmt->m_init);
    }

    m_pEnv->define(stmt->m_name, value);
    return nullptr;
}

std::any Interpreter::visitWhileStmt(stmt::While* stmt)
{
    while (isTruthy(evaluate(stmt->m_cond))) {
        execute(stmt->m_body);
    }
    return nullptr;
}

std::any Interpreter::visitReturnStmt(stmt::Return* stmt)
{
    std::any value = nullptr;
    if (stmt->m_value != nullptr) {
        value = evaluate(stmt->m_value);
    }

    throw NexReturn(value);
}

bool Interpreter::isEqual(std::any right, std::any left)
{
    if (auto pLeft = std::any_cast<std::nullptr_t>(&left))
    if (auto pRight = std::any_cast<std::nullptr_t>(&right)) {
        return true;
    }

    if (auto pLeft = std::any_cast<std::nullptr_t>(&left)) {
        return false;
    }

    if (auto pLeft = std::any_cast<double>(&left))
    if (auto pRight = std::any_cast<double>(&right)) {
        return *pLeft == *pRight;
    }

    if (auto pLeft = std::any_cast<std::wstring>(&left))
    if (auto pRight = std::any_cast<std::wstring>(&right)) {
        return *pLeft == *pRight;
    }

    return false;
}

std::any Interpreter::evaluate(std::shared_ptr<expr::Expr> e)
{
    return e->accept(this);
}

bool Interpreter::isTruthy(std::any value)
{
    if (std::any_cast<std::nullptr_t>(&value)) {
        return false;
    }

    if (auto pVal = std::any_cast<bool>(&value)) {
        return *pVal;
    }

    return true;
}

void Interpreter::checkNumberOperand(const Token& op, const std::any& operand)
{
    if (auto pOperand = std::any_cast<double>(&operand)) {
        return;
    }

    throw NexRunTimeError(op, L"Operand must be a number");
}

void Interpreter::checkNumberOperands(const Token& op,
                                      const std::any& left,
                                      const std::any& right)
{
    if (auto pLeft = std::any_cast<double>(&left))
    if (auto pRight = std::any_cast<double>(&right)) {
        return;
    }

    throw NexRunTimeError(op, L"Operands muse be numbers.");
}

std::wstring Interpreter::stringify(const std::any& value)
{
    std::wstringstream wss;
    if (auto num = std::any_cast<double>(&value)) {
        wss << *num;
    }
    else if (auto str = std::any_cast<std::wstring>(&value)) {
        wss << *str;
    
    }
    else if (auto pBoolean = std::any_cast<bool>(&value)) {
        if (*pBoolean) {
            wss << "true";
        }
        else {
            wss << "false";
        }
    }
    else if (auto callable = std::any_cast<std::shared_ptr<NexCallable>>(&value)) {
        wss << (*callable)->to_string();
    }
    else if (auto klass = std::any_cast<std::shared_ptr<NexClass>>(&value)) {
        wss << (*klass)->to_string();
    }
    else if (auto instance = std::any_cast<std::shared_ptr<NexInstance>>(&value)) {
        wss << (*instance)->to_string();
    }
    else {
        wss << "nil";
    }
    return wss.str();
}

void Interpreter::resolve(expr::Expr* expr, size_t idx)
{
    m_locals[expr] = idx;
}

std::any Interpreter::lookUpVariable(Token const& name, expr::Expr* expr)
{
    if (m_locals.count(expr)) {
        auto distance = m_locals[expr];
        return m_pEnv->getAt(distance, name.m_lexeme);
    }
    else {
        return m_pEnv->get(name);
    }
}

}

