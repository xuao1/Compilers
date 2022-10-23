# AST
本文档是对AST中的数据结构进行说明。

**源码链接：**

[SyntaxTree.h](../include/SyntaxTree.h)

[SyntaxTree.cpp](../src/SyntaxTree.cpp)


- [AST](#ast)
  - [Ptr](#ptr)
  - [PtrList](#ptrlist)
  - [Position](#position)
  - [Type](#type)
  - [Binop](#binop)
  - [UnaryOp](#unaryop)
  - [UnaryCondOp](#unarycondop)
  - [BinaryCondOp](#binarycondop)
  - [Node](#node)
  - [Assembly](#assembly)
  - [InitVal](#initval)
  - [GlobalDef](#globaldef)
  - [FuncDef](#funcdef)
  - [VarDef](#vardef)
  - [Stmt](#stmt)
  - [AssignStmt](#assignstmt)
  - [ReturnStmt](#returnstmt)
  - [BlockStmt](#blockstmt)
  - [EmptyStmt](#emptystmt)
  - [ExprStmt](#exprstmt)
  - [Expr](#expr)
  - [CondExpr](#condexpr)
  - [AddExpr](#addexpr)
  - [UnaryCondExpr](#unarycondexpr)
  - [BinaryCondExpr](#binarycondexpr)
  - [BinaryExpr](#binaryexpr)
  - [UnaryExpr](#unaryexpr)
  - [LVal](#lval)
  - [Literal](#literal)
  - [FuncCallStmt](#funccallstmt)
  - [FuncParam](#funcparam)
  - [FuncFParamList](#funcfparamlist)
  - [IfStmt](#ifstmt)
  - [WhileStmt](#whilestmt)
  - [BreakStmt](#breakstmt)
  - [ContinueStmt](#continuestmt)
  - [Visitor](#visitor)

## Ptr

AST中使用的指针类型。实际上是`std::shared_ptr`

## PtrList

存放[Ptr](#ptr)的list，实际上是`std::vector<Ptr>`

## Position

该节点代表的语法结构在源文件的位置信息，实际上是`yy::location`。由bison/flex自动生成。

## Type
包含SysYF语言支持的数据类型：`Type::INT`、`Type::VOID`以及`Type::FLOAT`

## Binop

双目算术表达式的操作符。包含

`Binop::PLUS` 加

`Binop::MINUS`减

`Binop::MULTIPLY`乘

`Binop::DIVIDE`除

`Binop::MODULO`模

## UnaryOp

单目算术表达式操作符，包含

`UnaryOp::PLUS`正

`UnaryOp::MINUS`负


## UnaryCondOp

单目条件表达式操作符，包含

`UnaryCondOp::NOT`非

## BinaryCondOp

双目条件表达式操作符，包含

`BinaryCondOp::LT`小于

`BinaryCondOp::LTE`小于等于

`BinaryCondOp::GT`大于

`BinaryCondOp::GTE`大于等于

`BinaryCondOp::EQ`等于等于

`BinaryCondOp::NEQ`不等于

`BinaryCondOp::LAND`逻辑与

`BinaryCondOp::LOR`逻辑或

## Node

语法树所有结点的基类，

`Node::loc`是其在对应源文件的位置信息。类型为[Position](#position)

`virtual void Node::accept(Visitor &visitor)`为虚函数，用于访问者模式。

## Assembly

AST的根结点

[PtrList](#ptrlist)<[GlobalDef](#globaldef)> `Assembly::global_defs`存放所有[GlobalDef](#globaldef)指针。

## InitVal

代表初值的结点。该结点为嵌套定义。以下类型的变量初值均可表示：

```c++
int a = 1 + 1;
int b[2] = {1,2};
int c[2][2] = {{1,2},{3,4}}
...
```

`bool InitVal::isExp`

为真时初值为[Expr](#expr)类型。为假时代表以`{...}`的形式进行赋初值
eg
```c++
int a = 3 + 1;//isExp=true
int c[2][2] = {{1,2},{3,4}};//isExp=false
```
所有`InitVal`结点最底层一定是[Expr](#expr)类型。也即`isExp`为true

[PtrList](#ptrlist)<[InitVal](#initval)> `InitVal::elementList`

当`isExp`为false时该域才有意义。是包含`{}`中其余`InitVal`结点指针的列表。

[Ptr](#ptr)<[Expr](#expr)> `InitVal::expr`

当`isExp`为true时该域才有意义。一个初值表达式的指针。

## GlobalDef

所有def结点的基类

## FuncDef

代表函数定义。

[Type](#type) `FuncDef::ret_type`。

函数的返回值类型

[Ptr](#ptr)<[FuncFParamList](#funcfparamlist)> `FuncDef::param_list`。

函数的形参指针

`std::string FuncDef::name` 

函数名

[Ptr](#ptr)<[BlockStmt](#blockstmt)> `FuncDef::body` 

函数体指针

## VarDef

代表变量定义

`bool VarDef::is_constant` 

是否为常量

[Type](#type) `VarDef::btype` 

变量类型(在SysYF中只能是int或float)

`std::string VarDef::name`

变量名

`bool VarDef::is_inited`

是否初始化

[PtrList](#ptrlist)<[Expr](#expr)>  `VarDef::array_length`

若为数组，则是存放各维长度表达式指针的列表，否则为空

[Ptr](#ptr)<[InitVal](#initval)> `VarDef::initializers`

若初始化，则是指向初值定义的指针

## Stmt

所有statement的基类

## AssignStmt

表示如下类型的语句:
```c++
target = value
```

即赋值型语句

[Ptr](#ptr)<[Lval](#lval)> `AssignStmt::target` 

赋值表达式的左值指针

[Ptr](#ptr)<[Expr](#expr)> `AssignStmt::value`

赋值表达式右边表达式指针

## ReturnStmt

代表return 语句

[Ptr](#ptr)<[Expr](#expr)> `ReturnStmt::ret`

return 语句返回的表达式指针。空指针代表void return

## BlockStmt

代表使用`{}`括起来的stmt。

[PtrList](#ptrlist)<[Stmt](#stmt)> `BlockStmt::body`

该block中所有stmt指针的列表

## EmptyStmt

空语句

## ExprStmt

表达式语句

[Ptr](#ptr)<[Expr](#exp)> `ExprStmt::exp`

表达式语句对应表达式的指针

## Expr

所有表达式的基类

## CondExpr

所有条件表达式的基类

## AddExpr

所有算术表达式的基类

## UnaryCondExpr

单目条件表达式

`UnaryCondOp UnaryCondExpr::op` 

操作符

[Ptr](#ptr)<[Expr](#expr)> `UnaryCondExpr::rhs`

操作符右端表达式指针

## BinaryCondExpr

双目条件表达式

`BinaryCondOp BinaryCondExpr::op` 

操作符

[Ptr](#ptr)<[Expr](#expr)> `BinaryCondExpr::lhs, rhs`

操作符左右两端表达式指针

## BinaryExpr

双目算术表达式

`BinOp BinaryExpr::op` 

操作符

[Ptr](#ptr)<[Expr](#expr)> `BinaryExpr::lhs, rhs`

操作符左右两端表达式指针

## UnaryExpr

单目算术表达式

`UnaryOp UnaryExpr::op` 

操作符

[Ptr](#ptr)<[Expr](#expr)> `UnaryExpr::rhs`

操作符右端表达式指针

## LVal

左值表达式

`std::string Lval::name` 

变量名

[PtrList](#ptrlist)<[Expr](#expr)> `LVal::array_index`

数组索引的指针列表。若不是数组，则为空

## Literal

语义值类型，包含整数和字符串

`Type Literal::literal_type`

语义值类型

`int Literal::int_const`

整数语义值

`double float_const`

浮点语义值

## FuncCallStmt

函数调用

`std::string FuncCallStmt::name` 

被调用的函数名

[PtrList](#ptrlist)<[Expr](#expr)> `FuncCallStmt::params`

存放函数实参表达式指针的列表

## FuncParam

单个函数形参

`std::string FuncParam::name` 

形参名

[Type](#type) `FuncParam::param_type` 

形参类型

[PtrList](#ptrlist)<[Expr](#expr)> `FuncParam::array_index`

形参的数组维度列表，存放每一维的表达式指针。若非数组则为空

## FuncFParamList

存放一个函数的所有形参

[PtrList](#ptrlist)<[FuncParam](#funcparam)> `FuncFParamList::params`

存放所有形参指针的列表

## IfStmt

表示如下结构:
```c++
if(cond_exp)
    if_stmt
或
if(cond_exp)
    if_stmt
else
    else_stmt
```

[Ptr](#ptr)<[Expr](#expr)> `IfStmt::cond_exp`

cond_exp的指针

[Ptr](#ptr)<[Stmt](#stmt)> `IfStmt::if_statement`

if_stmt的指针

[Ptr](#ptr)<[Stmt](#stmt)> `IfStmt::else_statement`

else_stmt的指针(若无else，则为空)


## WhileStmt

表示如下结构
```c++
while(cond_exp)
    stmt
```

[Ptr](#ptr)<[Expr](#expr)> `WhileStmt::cond_exp`

cond_exp的指针

[Ptr](#ptr)<[Stmt](#stmt)> `WhileStmt::statement`

stmt的指针


## BreakStmt

表示一个break语句

## ContinueStmt

表示一个continue语句

## Visitor

访问者模式的基类，用于访问AST。SyntaxTreePrinter就是基于此完成的。





选手您好，

您在 Hackergame 2022 平台上昵称为 Hana #mcfxfans 的账号经过裁判组调查核实，存在以下违规行为：

2022-10-22 18:31:45 此账号提交了其他选手在题目「HeiLang」中解出的 flag。

根据您的描述，是由于“使用学校邮箱注册账户时、因为邮箱延迟过高并且将验证码自动归类为spam、一度无法注册、遂使用floweyewe@gmail.com先行注册账户浏览题目”，经裁判组讨论决定，如果您可以验证floweyewe@gmail.com是您的邮箱，我们将予以解除封禁。

验证方式：使用上述 gmail 邮箱向 hackergame@ustclug.org 发送一封邮件，内容为您的校内邮箱和您在Hackergame 2022 平台上的昵称。

根据比赛规则（ https://hack.lug.ustc.edu.cn/terms/ ）第一条：“ 一人一个账号。无论出于何种理由和目的，以下行为均属于作弊行为：一人注册和使用多个账号参加比赛、多人共享一个账号参加比赛、登录他人账号”，一人注册使用多个账号是违规行为，我们将继续封禁注册邮箱为floweyewe@gmail.com的账号。 
祝好。



选手您好，

您在 Hackergame 2022 平台上昵称为 lyscf #bytessec #大佬们带带我 的账号经过裁判组调查核实，存在以下违规行为：

其他选手提交了此账号解出的 flag。

根据比赛规则（<https://hack.lug.ustc.edu.cn/terms/>），以上事实已经构成违规。如果您觉得上述行为不属实，请在回复中给出理由和相应证据。根据目前的我们掌握的信息，您的账号将保持封禁状态。

祝好。



选手您好，

您在 Hackergame 2022 平台上昵称为 134噶你腰子 的账号经过裁判组调查核实，存在以下违规行为：

其他选手提交了此账号解出的 flag。

根据比赛规则（<https://hack.lug.ustc.edu.cn/terms/>），以上事实已经构成违规。如果您觉得上述行为不属实，请在回复中给出理由和相应证据。根据目前的我们掌握的信息，您的账号将保持封禁状态。

祝好。





选手您好，
您在 Hackergame 2022 平台上昵称为 lyscf #bytessec #大佬们带带我 的账号经过裁判组调查核实，存在以下违规行为：
其他选手提交了此账号解出的 flag。
根据比赛规则（https://hack.lug.ustc.edu.cn/terms/），以上事实已经构成违规。如果您觉得上述行为不属实，请在回复中给出理由和相应证据。根据目前的我们掌握的信息，您的账号将保持封禁状态。
祝好。





选手您好，

您在 Hackergame 2022 平台上昵称为 lyscf #bytessec #大佬们带带我 的账号经过裁判组调查核实，存在以下违规行为：

2022-10-22 21:03:00 昵称为 ZianTT #mcfx fans 的选手提交了此账号在题目「HeiLang」解出的 flag。

根据比赛规则（ https://hack.lug.ustc.edu.cn/terms/ ），以上事实已经构成违规。如果您觉得上述行为不属实，请在回复中给出理由和相应证据。根据目前的我们掌握的信息，您的账号将保持封禁状态。

祝好。







选手您好，

您在 Hackergame 2022 平台上昵称为 bp233 的账号经过裁判组调查核实，存在以下违规行为：

2022-10-22 13:46:43 此账号提交了其他选手在题目「猫咪问答喵」中解出的 flag。

根据您的描述，是由于“发现注册的信息有误，想换一个号使用”。假设您的描述属实，裁判组注意到，10月23日早晨另有一账号提交了您第一个账号在题目 的 flag，

验证方式：向 [hackergame@ustclug.org](mailto:hackergame@ustclug.org) 发送一封邮件，内容为 昵称为 bp233 的账号的手机号，您之前账号的手机号。

根据比赛规则（https://hack.lug.ustc.edu.cn/terms/）第一条：“ 一人一个账号。无论出于何种理由和目的，以下行为均属于作弊行为：一人注册和使用多个账号参加比赛、多人共享一个账号参加比赛、登录他人账号”，一人注册使用多个账号是违规行为，我们将继续封禁您的第一个账号。
祝好。



```txt
选手 XM*****、XM*****、Do*****、Cr***** 4 人今日存在作弊或疑似作弊等违规行为，相关用户已经封禁，如有异议请通过平台底部邮件联系组委会申诉。提醒各位选手认真阅读诚信比赛承诺书、数据收集政策和参赛提示：https://hack.lug.ustc.edu.cn/terms/ 。
```


选手 Xe*****、ly*****、13*****、F3***** 等 35 人存在作弊或疑似作弊等违规行为，相关用户已经封禁，如有异议请通过平台底部邮件联系 组委会申诉。提醒各位选手认真阅读诚信比赛承诺书、数据收集政策和参赛提示：https://hack.lug.ustc.edu.cn/terms/ 。