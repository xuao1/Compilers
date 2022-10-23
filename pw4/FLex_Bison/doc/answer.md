## 问题 1-1

> 阅读 `demo` 中的 `main.cpp`、`demoDriver.cpp`、`demoDriver.h`，简要说明 ` Driver` 是如何工作的

`main.cpp`：处理命令行信息，设置好输出信息与工作模式（`-h`, `-p`, `-s`, `-emit-ast`）、文件名，初始化 `driver`、`printer` 和 `reproter`，调用 `driver.parser` 完成词法和语法分析，构件语法树并返回根节点，根据输入的命令决定是否调用 `accept(printer)` 完成通过 AST 还原代码

`demoDriver.h`：定义了 `demoDriver` 类，包括一些变量和函数

`demoDriver.cpp`：具体实现了这些函数。其中，`scan_begin()` 和 `scan_end()` 处理词法分析器的字符流来源，以及字符流的关闭；`parser` 为最主要的函数，它会调用 Bison 生成的语法分析器的主控函数，即 `yy:demoParser` 函数，进行词法和语法分析，取决于参数 `tarce_parsing`；`error()` 函数负责打印错误信息。 

综上所述，`Driver` 就是对已知的输入字符流，通过调用 Bison 生成的语法分析器（语法分析器中会调用 Flex 生成的词法分析器），完成语法和词法分析



## 问题 1-2

> 阅读并理解 demo 代码以及 Bison-Flex 协作的方式，目前的实现中并不强制 demo 语言程序包含 main 函数，而这不符合文档描述的语言规范。若在前端分析过程中就强制要求 demo 语言程序必须包含且只能包含一个 main 函数（也就是说把这个约束用语法定义来体现），应该如何修改词法语法定义？

#### 方法一

词法分析中，把 main 加入 keyword，放在规则部分，并且在规则部分中，至少要放在 Identifier 之前

```c++
main        {return yy::demoParser::make_MAIN(loc);}
```

在语法分析中，将 MAIN 加入 token

```c
%token INT RETURN VOID MAIN
```

在 Bison 中新建一个变量 `num_of_main`，初始化为 0，用来计数 main 函数的个数

在非终结符 Begin 的动作中加入：

```c++
    if(num_of_main != 1){error($1->loc, "error: must be one and only one main function.");}
```

再将 main 函数的规则加入 FunDef 的右部，即在原来的右部再加一个规则，用 | 隔开，与之前不同的是，之前规则识别的函数名是 Identifier，新加入的规则识别的是 main

```c++
  | VOID MAIN LPARENTHESE RPARENTHESE Block{
    $$ = new SyntaxTree::FuncDef();
    $$->ret_type = SyntaxTree::Type::VOID;
    $$->name = std::string("main");
    $$->body = SyntaxTree::Ptr<SyntaxTree::BlockStmt>($5);
    $$->loc = @$;
    num_of_main = num_of_main + 1;
  }
```

测试运行，使用到了 3 个测试代码，保存在 `demo\grammar`分别为：

`test.sy`：只包含一个 main 函数

`test1.sy`：不包含 main 函数，只有另一个函数 `fun()`

`test2.sy`：包含两个 main 函数

运行结果如下：

![image-20221021180213243](.\img\demo_result1.png)

#### 方法二

在运行了上述 `test2.sy` 后，我发现当前支持的语言是只能包含一个函数，而当我们加入限制“有且只有一个 main 函数“后，那么当前语言就是“只包含一个函数，而且函数为 main 函数”

那么可以采取另一种更为简便的修改方式。

词法分析的修改与方法一相同，把 main 加入 keyword，放在规则部分。

在语法分析中，将 MAIN 加入 token

然后无需引入 num_of_main，也无需修改 Begin 的秀东，只需修改 FunDef 的规则为：

```
FuncDef: VOID MAIN LPARENTHESE RPARENTHESE Block{
    $$ = new SyntaxTree::FuncDef();
    $$->ret_type = SyntaxTree::Type::VOID;
    $$->name = std::string("main");
    $$->body = SyntaxTree::Ptr<SyntaxTree::BlockStmt>($5);
    $$->loc = @$;
  };
```

如上修改后，再次生成并测试运行三个代码，结果如下：

![image-20221021181326337](.\img\demo_result2.png)

#### 注：

我提交的代码使用的是方法一
