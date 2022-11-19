#include "SyntaxTreeChecker.h"
#include <map>
#include <stack>

using namespace SyntaxTree;
std::stack<std::map<std::string, SyntaxTree::Type>> SymTable;
std::map<std::string, SyntaxTree::FuncFParamList> FuncParamTable;
int NumTable = 0; // 为处理函数声明和其他语句块的不同处理

void SyntaxTreeChecker::visit(Assembly& node) {
    std::map<std::string, SyntaxTree::Type> CurSymTable;
    SymTable.push(CurSymTable);
    NumTable++;
    for (auto def : node.global_defs) {
        def->accept(*this);
    }
    SymTable.pop();
    NumTable--;
}

// Type ret_type;
// Ptr<FuncFParamList> param_list;
// std::string name;
// Ptr<BlockStmt> body;
void SyntaxTreeChecker::visit(FuncDef& node) {
    auto temp = SymTable.top().find(node.name);
    if (temp != SymTable.top().end()) {
        err.error(node.loc, "Function duplicated.");
        exit(int(ErrorType::FuncDuplicated));
    }
    SymTable.top().insert(std::pair<std::string, SyntaxTree::Type>(node.name, node.ret_type));
    FuncParamTable.insert(std::pair<std::string, SyntaxTree::FuncFParamList>(node.name, *node.param_list));
    std::map<std::string, SyntaxTree::Type> CurSymTable;
    SymTable.push(CurSymTable);
    // 形式参数在函数体的顶层作用域内
    node.param_list->accept(*this);
    node.body->accept(*this);
    SymTable.pop();
    NumTable--;
}

// Exp 除了各类简单计算式，还有数字Literal，变量或数组元素LVal，以及函数调用FuncCallStmt
// BinOp op;
// Ptr<Expr> lhs, rhs;
void SyntaxTreeChecker::visit(BinaryExpr& node) {
    node.lhs->accept(*this);
    bool lhs_int = this->Expr_int;
    // std::cout << "visit rhs" << std::endl;
    node.rhs->accept(*this);
    bool rhs_int = this->Expr_int;
    if (node.op == SyntaxTree::BinOp::MODULO) {
        if (!lhs_int || !rhs_int) {
            err.error(node.loc, "Operands of modulo should be integers.");
            exit(int(ErrorType::Modulo));
        }
    }
    this->Expr_int = lhs_int & rhs_int;
}

// UnaryOp op;
// Ptr<Expr> rhs;
void SyntaxTreeChecker::visit(UnaryExpr& node) {
    node.rhs->accept(*this);
}

// std::string name;
// PtrList<Expr> array_index;
void SyntaxTreeChecker::visit(LVal& node) {
    std::stack<std::map<std::string, SyntaxTree::Type>> TempSymTable;
    while (!SymTable.empty()) {
        auto map1 = SymTable.top();
        auto temp = map1.find(node.name);
        if (temp != map1.end()) {
            this->Expr_int = (temp->second == SyntaxTree::Type::INT);
            break;
        }
        TempSymTable.push(map1);
        SymTable.pop();
        if (SymTable.empty()) {
            err.error(node.loc, "Variable unknow.");
            exit(int(ErrorType::VarUnknown));
        }
    }
    while (!TempSymTable.empty()) {
        auto map2 = TempSymTable.top();
        SymTable.push(map2);
        TempSymTable.pop();
    }
    for (auto exp : node.array_index) {
        exp->accept(*this);
    }
}

// Type literal_type;
// int int_const;
// double float_const;
void SyntaxTreeChecker::visit(Literal& node) {
    this->Expr_int = (node.literal_type == SyntaxTree::Type::INT);
}

// Ptr<Expr> ret;
void SyntaxTreeChecker::visit(ReturnStmt& node) {
    node.ret->accept(*this);
}

// bool is_constant;
// Type btype;
// std::string name;
// bool is_inited; // This is used to verify `{}`
// PtrList<Expr> array_length; // empty for non-array variables
// Ptr<InitVal> initializers;
void SyntaxTreeChecker::visit(VarDef& node) {
    if (node.is_inited) {
        node.initializers->accept(*this);
    }
    auto temp = SymTable.top().find(node.name);
    if (temp != SymTable.top().end()) {
        err.error(node.loc, "Variable duplicated.");
        exit(int(ErrorType::VarDuplicated));
    }
    for (auto exp : node.array_length) {
        exp->accept(*this);
    }
    SymTable.top().insert(std::pair<std::string, SyntaxTree::Type>(node.name, node.btype));
}

// Ptr<LVal> target;
// Ptr<Expr> value;
void SyntaxTreeChecker::visit(AssignStmt& node) {
    node.target->accept(*this);
    node.value->accept(*this);
}

// std::string name;
// PtrList<Expr> params;
void SyntaxTreeChecker::visit(FuncCallStmt& node) {  
    std::stack<std::map<std::string, SyntaxTree::Type>> TempSymTable;
    float flag = false;
    while (!SymTable.empty()) {
        auto map1 = SymTable.top();
        auto temp = map1.find(node.name);
        if (temp != map1.end()) {
            flag = (temp->second == SyntaxTree::Type::INT);
            break;
        }
        TempSymTable.push(map1);
        SymTable.pop();
        if (SymTable.empty()) {
            err.error(node.loc, "Function unknown.");
            exit(int(ErrorType::FuncUnknown));
        }
    }
    while (!TempSymTable.empty()) {
        auto map2 = TempSymTable.top();
        SymTable.push(map2);
        TempSymTable.pop();
    }

    /*
    struct FuncFParamList : Node
    {
        PtrList<FuncParam> params;
        void accept(Visitor& visitor) final;
    };
    */
    // 形参和实参的个数和类型需要完全匹配
    auto NowFunc = FuncParamTable.find(node.name);
    int length = NowFunc->second.params.size();
    if (node.params.size() == 0) {
        if (length) {
            err.error(node.loc, "FuncParams error.");
            exit(int(ErrorType::FuncParams));
        }
    }
    else {
        if (node.params.size() != length) {
            err.error(node.loc, "FuncParams error.");
            exit(int(ErrorType::FuncParams));
        }
        int i = 0;
        for (auto exp : node.params) {
            exp->accept(*this);
            if (NowFunc->second.params[i]->param_type == SyntaxTree::Type::INT && !this->Expr_int) {
                err.error(node.loc, "FuncParams error.");
                exit(int(ErrorType::FuncParams));
            }
            if (NowFunc->second.params[i]->param_type == SyntaxTree::Type::FLOAT && this->Expr_int) {
                err.error(node.loc, "FuncParams error.");
                exit(int(ErrorType::FuncParams));
            }
            i++;
        }
    }
    this->Expr_int = flag;
}

// PtrList<Stmt> body;
void SyntaxTreeChecker::visit(BlockStmt& node) {
    if (NumTable == SymTable.size()) {// 其他语句块
        std::map<std::string, SyntaxTree::Type> CurSymTable;
        SymTable.push(CurSymTable);
        NumTable++;
        for (auto stmt : node.body) {
            stmt->accept(*this);
        }
        SymTable.pop();
        NumTable--;
    }
    else { // 函数声明
        NumTable++;
        for (auto stmt : node.body) {
            stmt->accept(*this);
        }
    }
}

void SyntaxTreeChecker::visit(EmptyStmt& node) {}

// Ptr<Expr> exp;
void SyntaxTreeChecker::visit(SyntaxTree::ExprStmt& node) {
    node.exp->accept(*this);
}

// std::string name;
// Type param_type;
// PtrList<Expr> array_index; // nullptr if not indexed as array
void SyntaxTreeChecker::visit(SyntaxTree::FuncParam& node) {
    auto temp = SymTable.top().find(node.name);
    if (temp != SymTable.top().end()) {
        err.error(node.loc, "Variable duplicated.");
        exit(int(ErrorType::VarDuplicated));
    }
    for (auto exp : node.array_index) {
         exp->accept(*this);
    }
    SymTable.top().insert(std::pair<std::string, SyntaxTree::Type>(node.name, node.param_type));
}

// PtrList<FuncParam> params;
void SyntaxTreeChecker::visit(SyntaxTree::FuncFParamList& node) {
    for (auto parm : node.params) {
        parm->accept(*this);
    }
}

// BinaryCondOp op;
// Ptr<Expr> lhs, rhs;
void SyntaxTreeChecker::visit(SyntaxTree::BinaryCondExpr& node) {
    node.lhs->accept(*this);
    node.rhs->accept(*this);
}

// UnaryCondOp op;
// Ptr<Expr> rhs;
void SyntaxTreeChecker::visit(SyntaxTree::UnaryCondExpr& node) {
    node.rhs->accept(*this);
}

// Ptr<Expr> cond_exp;
// Ptr<Stmt> if_statement;
// Ptr<Stmt> else_statement;
void SyntaxTreeChecker::visit(SyntaxTree::IfStmt& node) {
    std::map<std::string, SyntaxTree::Type> CurSymTable0;
    SymTable.push(CurSymTable0);
    NumTable++;
    node.cond_exp->accept(*this);
    node.if_statement->accept(*this);
    SymTable.pop();
    NumTable--;
    if (node.else_statement) {
        std::map<std::string, SyntaxTree::Type> CurSymTable1;
        SymTable.push(CurSymTable1);
        NumTable++;
        node.else_statement->accept(*this);
        SymTable.pop();
        NumTable--;
    }
}

// Ptr<Expr> cond_exp;
// Ptr<Stmt> statement;
void SyntaxTreeChecker::visit(SyntaxTree::WhileStmt& node) {
    node.cond_exp->accept(*this);
    std::map<std::string, SyntaxTree::Type> CurSymTable;
    SymTable.push(CurSymTable);
    NumTable++;
    node.statement->accept(*this);
    SymTable.pop();
    NumTable--;
}

void SyntaxTreeChecker::visit(SyntaxTree::BreakStmt& node) {}

void SyntaxTreeChecker::visit(SyntaxTree::ContinueStmt& node) {}

// bool isExp;
// PtrList<InitVal> elementList;
// Ptr<Expr> expr;
void SyntaxTreeChecker::visit(SyntaxTree::InitVal& node) {
    if (node.isExp) {
        node.expr->accept(*this);
    } 
    else {
        for (auto element : node.elementList) {
            element->accept(*this);
        }
    }
}