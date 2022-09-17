### 分析与设计

题目要求熟悉 flex 的使用，编写 flex 的此法规范描述文件，参照 expr.c，编写 labLexer-2.c，其中需要对宏做判断。编写 Makefile，能够正确编译产生可执行文件。

首先是 relop.lex 的设计，参考题目给出的 expr.lex，可以熟悉 flex 词法描述文件的一般结构，然后根据此法要求进行编写。

然后是编写 labLexer-2.c，根据题目要求，在宏 `LEXERGEN` 定义时调用 Flex 生成的词法分析函数，在该宏未定义时调用第一关的函数。

该宏定义时，主要就是可以直接使用 `getsym()`，而宏未被定义时，这个函数需要自行编写。所以需要适当修改第一关的代码，完成 `getsym()` 函数的编写。

最后是要编写 Makefile 文件，参考 flex-example 子目录下的 Makefile。该文件主要是完成 flex 的生成以及 labLexer-2.c 的编译，基本上只需要修改 flex-example 中 Makefile 的具体文件名。

### 遇到的问题及解决方案

+ Flex 词法描述文件编写时，other 的表示

  因为 other 需要的不是正向描述，而是“非 XXX” 这样的含义。经查阅 Flex manual 可知：

  > [^A-Z\n]
  >
  > any character EXCEPT an uppercase letter or a newline

  所以可以用 ^ 来表示

+ 不明白 sym 是什么

  在 .lex 文件中没有找到 sym 的定义，但是根据头文件信息，可知他应该位于 pl0.h 中，阅读 pl0.h 发现，这是一个无符号长整形，所以我在编辑 .lex 文件时，返回值直接设置为了数字。

+ Makefile 编写时，如果只是修改参考文件的名字，会导致生成的 `lex.yy.c` 在与 Makefile 同级目录下，导致在下一步对他调用时编译器找不到此文件。

  经查阅资料，只需在 Makefile 中使用 cd 命令即可解决该问题。













