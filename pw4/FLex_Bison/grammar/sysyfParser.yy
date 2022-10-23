%skeleton "lalr1.cc" /* -*- c++ -*- */
%require "3.0"
%defines
//%define parser_class_name {sysyfParser}
%define api.parser.class {sysyfParser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires
{
#include <string>
#include "SyntaxTree.h"
class sysyfDriver;
}

// The parsing context.
%param { sysyfDriver& driver }

// Location tracking
%locations
%initial-action
{
// Initialize the initial location.
@$.begin.filename = @$.end.filename = &driver.file;
};

// Enable tracing and verbose errors (which may be wrong!)
%define parse.trace
%define parse.error verbose

// Parser needs to know about the driver:
%code
{
#include "sysyfDriver.h"
#define yylex driver.lexer.yylex
}

// Tokens:
%define api.token.prefix {TOK_}

%token END
/*********add your token here*********/
%token PLUS MINUS MULTIPLY DIVIDE MODULO
%token ASSIGN SEMICOLON
%token COMMA LPARENTHESE RPARENTHESE
%token LBRACE RBRACE
%token INT RETURN VOID FLOAT
%token <std::string>IDENTIFIER
%token <int>INTCONST
%token <float>FLOATCONST
%token EOL COMMENT
%token BLANK NOT
%token LT LTE GT GTE EQ NEQ 
%token LAND LOR
%token LBRACKET RBRACKET
%token CONST
%token IF ELSE WHILE BREAK CONTINUE


// Use variant-based semantic values: %type and %token expect genuine types
%type <SyntaxTree::Assembly*>CompUnit
%type <SyntaxTree::PtrList<SyntaxTree::GlobalDef>>GlobalDecl
/*********add semantic value definition here*********/
%type <SyntaxTree::Type>BType // 变量类型，包含 int 和 float

%type <SyntaxTree::FuncDef*>FuncDef
%type <SyntaxTree::PtrList<SyntaxTree::FuncParam>>FuncFParams // 函数形参表
%type <SyntaxTree::FuncParam*>FuncFParam // 函数形参

%type <SyntaxTree::PtrList<SyntaxTree::Expr>>Array // 数组（不包括类型和名字部分）

%type <SyntaxTree::BlockStmt*>Block
%type <SyntaxTree::PtrList<SyntaxTree::Stmt>>BlockItemList
%type <SyntaxTree::PtrList<SyntaxTree::Stmt>>BlockItem

%type <SyntaxTree::PtrList<SyntaxTree::VarDef>>Decl

%type <SyntaxTree::PtrList<SyntaxTree::VarDef>>ConstDecl
%type <SyntaxTree::PtrList<SyntaxTree::VarDef>>ConstDefList
%type <SyntaxTree::VarDef*>ConstDef

%type <SyntaxTree::PtrList<SyntaxTree::VarDef>>VarDecl
%type <SyntaxTree::PtrList<SyntaxTree::VarDef>>VarDefList
%type <SyntaxTree::VarDef*>VarDef

%type <SyntaxTree::InitVal*>InitVal
%type <SyntaxTree::PtrList<SyntaxTree::InitVal>>InitValList


%type <SyntaxTree::Stmt*>Stmt

%type <SyntaxTree::Expr*>Exp

%type <SyntaxTree::PtrList<SyntaxTree::Expr>>FuncRParams
%type <SyntaxTree::PtrList<SyntaxTree::Expr>>CommaExpList

%type <SyntaxTree::LVal*>LVal
%type <SyntaxTree::Literal*>Number

/*
%type <SyntaxTree::Expr*>Cond
%type <SyntaxTree::Expr*>LOrExp
%type <SyntaxTree::Expr*>LAndExp
%type <SyntaxTree::Expr*>EqExp
%type <SyntaxTree::Expr*>RelExp
*/


// 能用优先级，就用优先级解决。比如各种 Exp
// 完全按照 SysYF 的定义，会出现一堆问题

// No %destructors are needed, since memory will be reclaimed by the
// regular destructors.

// Grammar:
%start Begin 

%%
Begin: CompUnit END {
    $1->loc = @$;
    driver.root = $1;
    return 0;
  }
  ;

CompUnit:CompUnit GlobalDecl{
		$1->global_defs.insert($1->global_defs.end(), $2.begin(), $2.end());
		$$=$1;
	} 
	| GlobalDecl{
		$$=new SyntaxTree::Assembly();
		$$->global_defs.insert($$->global_defs.end(), $1.begin(), $1.end());
  }
	;

/*********add other semantic symbol definition here*********/
GlobalDecl:FuncDef{
    $$=SyntaxTree::PtrList<SyntaxTree::GlobalDef>();
    $$.push_back(SyntaxTree::Ptr<SyntaxTree::GlobalDef>($1));
  }
  | Decl{
    $$=SyntaxTree::PtrList<SyntaxTree::GlobalDef>();
    $$.insert($$.end(), $1.begin(), $1.end());
  }
  ;

BType: INT{
    $$=SyntaxTree::Type::INT;
  } 
  | FLOAT{
    $$=SyntaxTree::Type::FLOAT;
  }
  ;

FuncDef: BType IDENTIFIER LPARENTHESE FuncFParams RPARENTHESE Block{
    $$ = new SyntaxTree::FuncDef();
    $$->ret_type = $1;
    $$->name = $2;
    auto tmp = new SyntaxTree::FuncFParamList();
    tmp->params = $4;
    $$->param_list = SyntaxTree::Ptr<SyntaxTree::FuncFParamList>(tmp);
    $$->body = SyntaxTree::Ptr<SyntaxTree::BlockStmt>($6);
    $$->loc = @$;
  }
  | VOID IDENTIFIER LPARENTHESE FuncFParams RPARENTHESE Block{
    $$ = new SyntaxTree::FuncDef();
    $$->ret_type = SyntaxTree::Type::VOID;
    $$->name = $2;
    auto tmp = new SyntaxTree::FuncFParamList();
    tmp->params = $4;
    $$->param_list = SyntaxTree::Ptr<SyntaxTree::FuncFParamList>(tmp);
    $$->body = SyntaxTree::Ptr<SyntaxTree::BlockStmt>($6);
    $$->loc = @$;
  }
  ;


// FuncFParams → FuncFParam { ',' FuncFParam } 
FuncFParams: FuncFParam{
    $$ = SyntaxTree::PtrList<SyntaxTree::FuncParam>();
    $$.push_back(SyntaxTree::Ptr<SyntaxTree::FuncParam>($1));
  }
  | FuncFParams COMMA FuncFParam{
    $1.push_back(SyntaxTree::Ptr<SyntaxTree::FuncParam>($3));
    $$ = $1;
  }
  | %empty{
    $$ = SyntaxTree::PtrList<SyntaxTree::FuncParam>();
  }
  ;

// FuncFParam → BType Ident ['[' ']' { '[' Exp ']' }]
FuncFParam: BType IDENTIFIER{
    $$ = new SyntaxTree::FuncParam();
    $$->param_type = $1;
    $$->name = $2;
    $$->loc = @$;
  }
  | BType IDENTIFIER LBRACKET RBRACKET{
    $$ = new SyntaxTree::FuncParam();
    $$->param_type = $1;
    $$->name = $2;
    $$->array_index.insert($$->array_index.begin(),NULL);
    $$->loc = @$;
  }
  | BType IDENTIFIER LBRACKET RBRACKET Array{
    $$ = new SyntaxTree::FuncParam();
		$$->param_type = $1;
		$$->name = $2;
		$$->array_index = $5;
		$$->array_index.insert($$->array_index.begin(),NULL);
		$$->loc = @$;
  }
  ;

Array: LBRACKET Exp RBRACKET{
    $$=SyntaxTree::PtrList<SyntaxTree::Expr>();
		$$.push_back(SyntaxTree::Ptr<SyntaxTree::Expr>($2));
  }
  | Array LBRACKET Exp RBRACKET{
    $1.push_back(SyntaxTree::Ptr<SyntaxTree::Expr>($3));
		$$=$1;
  }
  ;

/*
// Exp → AddExp
Exp: AddExp{
    $$ = $1;
  }
  ;

AddExp: MulExp{
    $$ = $1;
  } 
  | AddExp PLUS MulExp{
    auto tmp=new SyntaxTree::BinaryExpr();
    tmp->op = SyntaxTree::BinOp::PLUS;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  | AddExp MINUS MulExp{
    auto tmp=new SyntaxTree::BinaryExpr();
    tmp->op = SyntaxTree::BinOp::MINUS;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  ;

// MulExp → UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
MulExp: UnaryExp{
    $$ = $1; 
  }
  | MulExp MULTIPLY UnaryExp{
    auto tmp=new SyntaxTree::BinaryExpr();
    tmp->op = SyntaxTree::BinOp::MULTIPLY;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  | MulExp DIVIDE UnaryExp{
    auto tmp=new SyntaxTree::BinaryExpr();
    tmp->op = SyntaxTree::BinOp::DIVIDE;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  | MulExp MODULO UnaryExp{
    auto tmp=new SyntaxTree::BinaryExpr();
    tmp->op = SyntaxTree::BinOp::MODULO;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  ;

//UnaryExp →
//PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
UnaryExp: PrimaryExp{
    $$ = $1;
  }
  | IDENTIFIER LPARENTHESE FuncRParams RPARENTHESE{
    auto tmp = new SyntaxTree::FuncCallStmt();
    tmp->name = $1;
    tmp->params = $3;
    $$->loc = @$;
    $$ = tmp;
  } 
  | IDENTIFIER LPARENTHESE RPARENTHESE{
    auto tmp = new SyntaxTree::FuncCallStmt();
    tmp->name = $1;
    tmp->params = SyntaxTree::PtrList<SyntaxTree::Expr>();
    $$->loc = @$;
    $$ = tmp;
  }
  | PLUS UnaryExp{
    auto tmp = new SyntaxTree::UnaryExpr();
    tmp->op = SyntaxTree::UnaryOp::PLUS;
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($2);
		$$->loc = @$;
    $$ = tmp;
  }
  | MINUS UnaryExp{
    auto tmp = new SyntaxTree::UnaryExpr();
    tmp->op = SyntaxTree::UnaryOp::MINUS;
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($2);
		$$->loc = @$;
    $$ = tmp;
  }
  ;

// PrimaryExp → '(' Exp ')' | LVal | Number
PrimaryExp: LPARENTHESE Exp RPARENTHESE{
    $$ = $2;
  }
  | LVal{
    $$ = $1; 
  }
  | Number{
    $$ = $1;
  }
  ;

*/

%left LOR;
%left LAND;
%left EQ NEQ;
%left LT LTE GT GTE;
%left PLUS MINUS;
%left MULTIPLY DIVIDE MODULO;
%precedence UPLUS UMINUS UNOT;
Exp:PLUS Exp %prec UPLUS {
		auto tmp = new SyntaxTree::UnaryExpr();
		tmp->op = SyntaxTree::UnaryOp::PLUS;
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($2);
		$$ = tmp;
		$$->loc = @$;
  }
  | MINUS Exp %prec UMINUS{
		auto tmp = new SyntaxTree::UnaryExpr();
		tmp->op = SyntaxTree::UnaryOp::MINUS;
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($2);
		$$ = tmp;
		$$->loc = @$;
	}
  | Exp LT Exp {
    auto temp = new SyntaxTree::BinaryCondExpr();
    temp->op = SyntaxTree::BinaryCondOp::LT;
    temp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    temp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
}
| Exp GT Exp {
    auto temp = new SyntaxTree::BinaryCondExpr();
    temp->op = SyntaxTree::BinaryCondOp::GT;
    temp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    temp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
} 
| Exp LTE Exp {
    auto temp = new SyntaxTree::BinaryCondExpr();
    temp->op = SyntaxTree::BinaryCondOp::LTE;
    temp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    temp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
}
| Exp GTE Exp {
    auto temp = new SyntaxTree::BinaryCondExpr();
    temp->op = SyntaxTree::BinaryCondOp::GTE;
    temp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    temp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
}
| Exp EQ Exp {
    auto temp = new SyntaxTree::BinaryCondExpr();
    temp->op = SyntaxTree::BinaryCondOp::EQ;
    temp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    temp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
}
| Exp NEQ Exp {
    auto temp = new SyntaxTree::BinaryCondExpr();
    temp->op = SyntaxTree::BinaryCondOp::NEQ;
    temp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    temp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
}
| Exp LAND Exp {
    auto temp = new SyntaxTree::BinaryCondExpr();
    temp->op = SyntaxTree::BinaryCondOp::LAND;
    temp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    temp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
}
| Exp LOR Exp {
    auto temp = new SyntaxTree::BinaryCondExpr();
    temp->op = SyntaxTree::BinaryCondOp::LOR;
    temp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    temp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = temp;
    $$->loc = @$;
}
	| NOT Exp %prec UNOT{
		auto tmp = new SyntaxTree::UnaryCondExpr();
		tmp->op = SyntaxTree::UnaryCondOp::NOT;
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($2);
		$$ = tmp;
		$$->loc = @$;
	}
	| Exp PLUS Exp {
		auto tmp = new SyntaxTree::BinaryExpr();
		tmp->op = SyntaxTree::BinOp::PLUS;
		tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
		$$ = tmp;
		$$->loc = @$;
	}
	| Exp MINUS Exp{
		auto tmp = new SyntaxTree::BinaryExpr();
		tmp->op = SyntaxTree::BinOp::MINUS;
		tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
		$$ = tmp;
		$$->loc = @$;
	}
  | Exp MULTIPLY Exp{
		auto tmp = new SyntaxTree::BinaryExpr();
		tmp->op = SyntaxTree::BinOp::MULTIPLY;
		tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
		$$ = tmp;
		$$->loc = @$;
	}
	| Exp DIVIDE Exp{
		auto tmp = new SyntaxTree::BinaryExpr();
		tmp->op = SyntaxTree::BinOp::DIVIDE;
		tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
		$$ = tmp;
		$$->loc = @$;
	}
	| Exp MODULO Exp{
		auto tmp = new SyntaxTree::BinaryExpr();
		tmp->op = SyntaxTree::BinOp::MODULO;
		tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
		tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
		$$ = tmp;
		$$->loc = @$;
  }
	| IDENTIFIER LPARENTHESE FuncRParams RPARENTHESE{
		auto tmp = new SyntaxTree::FuncCallStmt();
		tmp->name = $1;
		tmp->params = $3;
		$$ = tmp;
		$$->loc = @$;
	}
	| LPARENTHESE Exp RPARENTHESE{
    $$ = $2;
  }
	| LVal{
    $$ = $1;
  }
	| Number{
    $$ = $1;
  }
	;


// LVal → Ident {'[' Exp ']'}
LVal: IDENTIFIER{
    $$ = new SyntaxTree::LVal();
    $$->name = $1;
    $$->loc = @$;
  }
  | IDENTIFIER Array{
    $$ = new SyntaxTree::LVal();
    $$->name = $1;
    $$->array_index = $2;
    $$->loc = @$;
  }
  ;

// Number → IntConst | FloatConst
Number: INTCONST{
    $$ = new SyntaxTree::Literal();
    $$->literal_type = SyntaxTree::Type::INT;
    $$->int_const = $1;
    $$->loc = @$;
  }
  | FLOATCONST{
    $$ = new SyntaxTree::Literal();
    $$->literal_type = SyntaxTree::Type::FLOAT;
    $$->float_const = $1;
    $$->loc = @$;
  }
  ;

// Block → '{' { BlockItem } '}'
Block: LBRACE BlockItemList RBRACE{
    $$ = new SyntaxTree::BlockStmt();
    $$->body = $2;
    $$->loc = @$;
  }
  ;

BlockItemList: BlockItemList BlockItem {
    $1.insert($1.end(), $2.begin(), $2.end());
    $$ = $1;
  }
  | %empty {
    $$ = SyntaxTree::PtrList<SyntaxTree::Stmt>();
  };

// BlockItem → Decl | Stmt
BlockItem: Decl{
    $$ = SyntaxTree::PtrList<SyntaxTree::Stmt>();
		$$.insert($$.end(), $1.begin(), $1.end());
  }
  | Stmt{
    $$ = SyntaxTree::PtrList<SyntaxTree::Stmt>();
    $$.push_back(SyntaxTree::Ptr<SyntaxTree::Stmt>($1));
  }
  ;

// Decl → ConstDecl | VarDecl
Decl: ConstDecl{
    $$ = $1;
  }
  | VarDecl{
    $$ = $1;
  }
  ;

//ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
ConstDecl: CONST BType ConstDefList SEMICOLON{
    $$ = $3;
    for(auto &node : $$) {
        node->btype = $2;
        node->is_constant = true;
    }
  }
  ;

ConstDefList: ConstDefList COMMA ConstDef{
    $1.push_back(SyntaxTree::Ptr<SyntaxTree::VarDef>($3));
		$$=$1;
  }
  | ConstDef{
    $$=SyntaxTree::PtrList<SyntaxTree::VarDef>();
		$$.push_back(SyntaxTree::Ptr<SyntaxTree::VarDef>($1));
  }
  ;

// ConstDef → Ident { '[' ConstExp ']' } '=' ConstInitVal
// Exp 和 ConstExp 右部都是 AddExp，所以不做区分
/* 
struct VarDef : Stmt, GlobalDef
{
    bool is_constant;
    Type btype;
    std::string name;
    bool is_inited; // This is used to verify `{}`
    PtrList<Expr> array_length; // empty for non-array variables
    Ptr<InitVal> initializers;
    void accept(Visitor &visitor) final;
};
*/
ConstDef: IDENTIFIER Array ASSIGN InitVal{
    $$ = new SyntaxTree::VarDef();
    $$->is_constant = true;
    $$->name = $1;
    $$->is_inited = true;
    $$->array_length = $2;
    $$->initializers = SyntaxTree::Ptr<SyntaxTree::InitVal>($4);
    $$->loc = @$;
  }  
  | IDENTIFIER ASSIGN InitVal{
    $$ = new SyntaxTree::VarDef();
    $$->is_constant = true;
    $$->name = $1;
    $$->is_inited = true;
    $$->initializers = SyntaxTree::Ptr<SyntaxTree::InitVal>($3);
    $$->loc = @$;
  }
  ;

// VarDecl → BType VarDef { ',' VarDef } ';'
VarDecl: BType VarDefList SEMICOLON{
    $$ = $2;
    for(auto &node : $$) {
        node->btype = $1;
        node->is_constant = false;
    }
  }
  ;

VarDefList: VarDefList COMMA VarDef{
    $1.push_back(SyntaxTree::Ptr<SyntaxTree::VarDef>($3));
		$$=$1;
  }
  | VarDef{
    $$=SyntaxTree::PtrList<SyntaxTree::VarDef>();
		$$.push_back(SyntaxTree::Ptr<SyntaxTree::VarDef>($1));
  }
  ;

// VarDef →
//  Ident { '[' ConstExp ']' }
//  | Ident { '[' ConstExp ']' } '=' InitVal
VarDef: IDENTIFIER Array ASSIGN InitVal{
    $$ = new SyntaxTree::VarDef();
    $$->is_constant = false;
    $$->name = $1;
    $$->is_inited = true;
    $$->array_length = $2;
    $$->initializers = SyntaxTree::Ptr<SyntaxTree::InitVal>($4);
    $$->loc = @$;
  }
  | IDENTIFIER ASSIGN InitVal{
    $$ = new SyntaxTree::VarDef();
    $$->is_constant = false;
    $$->name = $1;
    $$->is_inited = true;
    $$->initializers = SyntaxTree::Ptr<SyntaxTree::InitVal>($3);
    $$->loc = @$;
  }
  | IDENTIFIER Array{
    $$ = new SyntaxTree::VarDef();
    $$->is_constant = false;
    $$->name = $1;
    $$->is_inited = false;
    $$->array_length = $2;
    $$->initializers = SyntaxTree::Ptr<SyntaxTree::InitVal>();
    $$->loc = @$;
  }
  | IDENTIFIER{
    $$ = new SyntaxTree::VarDef();
    $$->is_constant = false;
    $$->name = $1;
    $$->is_inited = false;
    $$->initializers = SyntaxTree::Ptr<SyntaxTree::InitVal>();
    $$->loc = @$;
  }
  ;

// ConstInitVal → ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
// InitVal → Exp | '{' [ InitVal { ',' InitVal } ] '}'
/*
struct InitVal: Node{
    bool isExp;
    PtrList<InitVal> elementList;
    Ptr<Expr> expr;
    void accept(Visitor &visitor) final;
};
*/
InitVal: Exp{
    $$ = new SyntaxTree::InitVal();
    $$->isExp = true;
    $$->loc = @$;
    $$->expr = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
  }
  | LBRACE RBRACE{
    $$ = new SyntaxTree::InitVal();
    $$->isExp = false;
    $$->loc = @$;
  }
  | LBRACE InitValList RBRACE{
    $$ = new SyntaxTree::InitVal();
    $$->isExp = false;
    $$->elementList.insert($$->elementList.end(), $2.begin(), $2.end());
    $$->loc = @$;
  }
  ;

InitValList: InitValList COMMA InitVal{
    $1.push_back(SyntaxTree::Ptr<SyntaxTree::InitVal>($3));
    $$ = $1;
  }
  | InitVal{
    $$ = SyntaxTree::PtrList<SyntaxTree::InitVal>();
    $$.push_back(SyntaxTree::Ptr<SyntaxTree::InitVal>($1));
  }
  ;

/*
Stmt
→
LVal '=' Exp ';' | [Exp] ';' | Block
| 'if' '( Cond ')' Stmt [ 'else' Stmt ]
| 'while' '(' Cond ')' Stmt
| 'break' ';' | 'continue' ';'
| 'return' [Exp] ';'
*/

%precedence RPARENTHESE;
%precedence ELSE; // 解决else最近匹配问题
Stmt: LVal ASSIGN Exp SEMICOLON{
    auto tmp = new SyntaxTree::AssignStmt();
    tmp->target = SyntaxTree::Ptr<SyntaxTree::LVal>($1);
    tmp->value = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$ = tmp;
    $$->loc = @$;
  }
  | Exp SEMICOLON{
    auto tmp = new SyntaxTree::ExprStmt();
    tmp->exp = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    $$ = tmp;
    $$->loc = @$;
  }
  | SEMICOLON{
    $$ = new SyntaxTree::EmptyStmt();
    $$->loc = @$;
  }
  | Block{
    $$ = $1;
  }
  | IF LPARENTHESE Exp RPARENTHESE Stmt{
    auto tmp = new SyntaxTree::IfStmt();
    tmp->cond_exp = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    tmp->if_statement = SyntaxTree::Ptr<SyntaxTree::Stmt>($5);
    $$ = tmp;
    $$->loc = @$;
  }
  | IF LPARENTHESE Exp RPARENTHESE Stmt ELSE Stmt{
    auto tmp = new SyntaxTree::IfStmt();
    tmp->cond_exp = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    tmp->if_statement = SyntaxTree::Ptr<SyntaxTree::Stmt>($5);
    tmp->else_statement = SyntaxTree::Ptr<SyntaxTree::Stmt>($7);
    $$ = tmp;
    $$->loc = @$;
  } 
  | WHILE LPARENTHESE Exp RPARENTHESE Stmt{
    auto tmp = new SyntaxTree::WhileStmt();
    tmp->cond_exp = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    tmp->statement = SyntaxTree::Ptr<SyntaxTree::Stmt>($5);
    $$ = tmp;
    $$->loc = @$;
  }
  | BREAK SEMICOLON{
    $$ = new SyntaxTree::BreakStmt();
    $$->loc = @$;
  }
  | CONTINUE SEMICOLON{
    $$ = new SyntaxTree::ContinueStmt();
    $$->loc = @$;
  }
  | RETURN SEMICOLON{
    auto tmp = new SyntaxTree::ReturnStmt();
    tmp->ret = SyntaxTree::Ptr<SyntaxTree::Expr>();
    $$ = tmp;
    $$->loc = @$;
  }
  | RETURN Exp SEMICOLON{
    auto tmp = new SyntaxTree::ReturnStmt();
    tmp->ret = SyntaxTree::Ptr<SyntaxTree::Expr>($2);
    $$ = tmp;
    $$->loc = @$;
  }
  ;


/*
// Cond → LOrExp
Cond: LOrExp{
    $$ = $1;
  }
  ;

// LOrExp → LAndExp | LOrExp '||' LAndExp
/*
struct BinaryCondExpr : CondExpr{
    BinaryCondOp op;
    Ptr<Expr> lhs,rhs;
    void accept(Visitor &visitor) final;
};
*/
/*
LOrExp: LAndExp{
    $$ = $1;
  }
  | LOrExp LOR LAndExp{
    auto tmp = new SyntaxTree::BinaryCondExpr();
    tmp->op = SyntaxTree::BinaryCondOp::LOR;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  ;

// LAndExp → EqExp | LAndExp '&&' EqExp
LAndExp: EqExp{
    $$ = $1;
  }
  | LAndExp LAND EqExp{
    auto tmp = new SyntaxTree::BinaryCondExpr();
    tmp->op = SyntaxTree::BinaryCondOp::LAND;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  ;

// EqExp → RelExp | EqExp ('==' | '!=') RelExp
EqExp: RelExp{
    $$ = $1;
  }
  | EqExp EQ RelExp{
    auto tmp = new SyntaxTree::BinaryCondExpr();
    tmp->op = SyntaxTree::BinaryCondOp::EQ;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  | EqExp NEQ RelExp{
    auto tmp = new SyntaxTree::BinaryCondExpr();
    tmp->op = SyntaxTree::BinaryCondOp::NEQ;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  ;

// RelExp → AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
RelExp: Exp{
    $$ = $1;
  }
  | RelExp LT Exp{
    auto tmp = new SyntaxTree::BinaryCondExpr();
    tmp->op = SyntaxTree::BinaryCondOp::LT;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  | RelExp GT Exp{
    auto tmp = new SyntaxTree::BinaryCondExpr();
    tmp->op = SyntaxTree::BinaryCondOp::GT;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  | RelExp LTE Exp{
    auto tmp = new SyntaxTree::BinaryCondExpr();
    tmp->op = SyntaxTree::BinaryCondOp::LTE;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  | RelExp GTE Exp{
    auto tmp = new SyntaxTree::BinaryCondExpr();
    tmp->op = SyntaxTree::BinaryCondOp::GTE;
    tmp->lhs = SyntaxTree::Ptr<SyntaxTree::Expr>($1);
    tmp->rhs = SyntaxTree::Ptr<SyntaxTree::Expr>($3);
    $$->loc = @$;
    $$ = tmp;
  }
  ;
*/
/*
// FuncRParams → Exp { ',' Exp }
FuncRParams: Exp{
    $$ = SyntaxTree::PtrList<SyntaxTree::Expr>();
    $$.push_back(SyntaxTree::Ptr<SyntaxTree::Expr>($1));
  }
  | FuncRParams COMMA Exp{
    $1.push_back(SyntaxTree::Ptr<SyntaxTree::Expr>($3));
    $$ = $1;
  }
  ;
*/

FuncRParams: CommaExpList Exp{
	  $1.push_back(SyntaxTree::Ptr<SyntaxTree::Expr>($2));
	  $$ = $1;
	}
	| %empty{
	  $$ = SyntaxTree::PtrList<SyntaxTree::Expr>();
	}
	;

CommaExpList:CommaExpList Exp COMMA{
    $1.push_back(SyntaxTree::Ptr<SyntaxTree::Expr>($2));
	  $$ = $1;
	}
	| %empty{
	  $$ = SyntaxTree::PtrList<SyntaxTree::Expr>();
	}
	;

%%

// Register errors to the driver:
void yy::sysyfParser::error (const location_type& l,
                          const std::string& m)
{
    driver.error(l, m);
}
