#ifndef _SYSYF_SYNTAX_TREE_CHECKER_H_
#define _SYSYF_SYNTAX_TREE_CHECKER_H_

#include <cassert>
#include "ErrorReporter.h"
#include "SyntaxTree.h"

class SyntaxTreeChecker : public SyntaxTree::Visitor {
   public:
    SyntaxTreeChecker(ErrorReporter& e) : err(e) {}
    virtual void visit(SyntaxTree::Assembly& node) override;
    virtual void visit(SyntaxTree::FuncDef& node) override;
    virtual void visit(SyntaxTree::BinaryExpr& node) override;
    virtual void visit(SyntaxTree::UnaryExpr& node) override;
    virtual void visit(SyntaxTree::LVal& node) override;
    virtual void visit(SyntaxTree::Literal& node) override;
    virtual void visit(SyntaxTree::ReturnStmt& node) override;
    virtual void visit(SyntaxTree::VarDef& node) override;
    virtual void visit(SyntaxTree::AssignStmt& node) override;
    virtual void visit(SyntaxTree::FuncCallStmt& node) override;
    virtual void visit(SyntaxTree::BlockStmt& node) override;
    virtual void visit(SyntaxTree::EmptyStmt& node) override;
    virtual void visit(SyntaxTree::ExprStmt& node) override;
    virtual void visit(SyntaxTree::FuncParam& node) override;
    virtual void visit(SyntaxTree::FuncFParamList& node) override;
    virtual void visit(SyntaxTree::BinaryCondExpr& node) override;
    virtual void visit(SyntaxTree::UnaryCondExpr& node) override;
    virtual void visit(SyntaxTree::IfStmt& node) override;
    virtual void visit(SyntaxTree::WhileStmt& node) override;
    virtual void visit(SyntaxTree::BreakStmt& node) override;
    virtual void visit(SyntaxTree::ContinueStmt& node) override;
    virtual void visit(SyntaxTree::InitVal& node) override;

   private:
    ErrorReporter& err;
    bool Expr_int;
};

enum class ErrorType {
    Accepted = 0,
    Modulo,
    VarUnknown,
    VarDuplicated,
    FuncUnknown,
    FuncDuplicated,
    FuncParams
};

#endif  // _SYSYF_SYNTAX_TREE_CHECKER_H_