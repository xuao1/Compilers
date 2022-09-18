### 分析与设计

题目要求熟悉 antlr 的使用，针对第一关的关系运算符的识别需求，用 antlr4 自动生成相应的词法分析器源码并编译和测试。

首先是编译运行 `antlr4-examples ` 子目录下的例子，参考 README.md 文件，可成功运行。

参照 `antlr4-examples ` 下的 g4 文件，编写第一关要求的词法规范描述文件。 其正确性可以由词法描述规则保证。

参照`antlr4-examples`子目录下的`main.cpp`，编写`labLexer-3.cpp`，输入输出与前两关是一样的。

编写`CMakeLists.txt`以及测试脚本，生成的最终可执行程序为`labLexer-3`

 

### 遇到的问题及解决方案

+ 与第二关一样，关于 other 的表述：

  经查阅资料可知：

| ~x   | Match any single character not in the set described by x. Set x can be a single character literal, a range, or a subrule set like ~('x'\|'y'\|'z') or ~[xyz]. Here is a rule that uses ~ to match any character other than characters using ~[\r\n]*:` 	 COMMENT : '#' ~[\r\n]* '\r'? '\n' -> skip ;` |
| ---- | ------------------------------------------------------------ |

​	所以可以用 ~ 来表示

+ 关于 token 的函数：

  希望使用到 token 返回值的具体内容和长度。经查找 antlr4-runtime 的 Token.h 库，发现了这两个函数：

```c++
/// Get the text of the token.
virtual std::string getText() const = 0;

/// Get the token type of the token
virtual size_t getType() const = 0;
```











