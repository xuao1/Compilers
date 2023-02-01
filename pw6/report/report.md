# PW6 实验报告

## 问题回答

### 1-1 

> 请给出 while 语句对应的 LLVM IR 的代码布局特点，重点解释其中涉及的几个`br`指令的含义（包含各个参数的含义）

用 `clang` 指令生成的关于 `while_test.sy` 的 LLVM IR 为：

```assembly
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, i32* %1, align 4			
  store i32 0, i32* %2, align 4			; b
  store i32 3, i32* @a, align 4
  br label %3							; 无条件转移到while循环的条件判断语句

3: ; 条件判断语句                                   ; preds = %11, %0
  %4 = load i32, i32* @a, align 4
  %5 = icmp sgt i32 %4, 0				; a>0
  br i1 %5, label %6, label %9			; 若a>0,跳转到 label 6 判断第二个条件，否则直接到 label 9

6: ; a>0                                          ; preds = %3
  %7 = load i32, i32* %2, align 4
  %8 = icmp slt i32 %7, 10				; b<10
  br label %9							; 无条件跳转到 label9，因为在 label 9 中会根据条件成立与否决定while循环体是否执行

9:                                                ; preds = %6, %3
  %10 = phi i1 [ false, %3 ], [ %8, %6 ]	; phi 指令
  br i1 %10, label %11, label %17		; a>0且b<10,跳至循环体11，否则至17

11: ;while体                                       ; preds = %9
  %12 = load i32, i32* %2, align 4
  %13 = load i32, i32* @a, align 4
  %14 = add nsw i32 %12, %13
  store i32 %14, i32* %2, align 4
  %15 = load i32, i32* @a, align 4
  %16 = sub nsw i32 %15, 1
  store i32 %16, i32* @a, align 4
  br label %3							;执行完while体，无条件转移至条while的件判断语句

17: ;while的下一句                                   ; preds = %9
  %18 = load i32, i32* %1, align 4
  ret i32 %18
}
```

while 循环部分的 c 代码为：

```
while (a > 0 && b < 10) {
		b = b + a;
		a = a - 1;
	}
```

+ 代码布局特点：进入 while 循环，首先进入条件判断语句 label3，由于有两个条件，且为“与”，所以如果第一个条件不成立，直接到 label9，若第一个成立，再进入 label 6 判断第二个条件。判断完后都要进入 label9，label 9 中使用到了 phi 指令，是根据两个条件的结果决定跳转，若条件成立，进入循环体 label 11，否则结束 whle 循环。

+ 解释各条 `br` 指令：已在上面的代码中加入了相应的注释

### 1-2 

> 请简述函数调用语句对应的 LLVM IR 的代码特点

调用前，会先将要传递的参数保存在寄存器中，之后用 `call` 指令调用相应函数，如：

```assembly
%11 = call i32 @climbStairs(i32 %10) 
```

等式右边，第一个参数为返回值类型，第二个参数是函数名，函数名前需要加 @，括号内为要传递的参数。

函数返回值赋值给等式左边的寄存器。

### 2-1

> 请给出`SysYFIR.md`中提到的两种getelementptr用法的区别, 并解释原因:
>
> - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0`
> - `%2 = getelementptr i32, i32* %1, i32 %0`

第一种是用来获得数组元素的地址值，对于第一个 0，是计算 %1 所指向的数组的第一个元素，第二个 0 计算该元素偏移 0 个 i32 的元素，即该元素本身，所以第一条指令返回首元素的地址值。

第二种指令，偏移 0 是计算 %1 指向的变量，即数组的指针，然后返回该指针的地址。

### 3-1

>  `src/driver.cpp`中`printASTUnit()`方法通过编译器实例`_Clang`执行`ASTDumpAction`，实现了解析抽象语法树并打印的功能。请根据 Clang driver处理流程回答该方法包含了哪几个阶段？

`printASTUnit()`方法是实现解析抽象语法树并打印，其代码为：

```c++
 if(_show_AST){
        _Clang.getFrontendOpts().ASTDumpAll = true;
        std::unique_ptr<ASTDumpAction> AAct(new clang::ASTDumpAction());
        _Clang.ExecuteAction(*AAct);
        _Clang.getFrontendOpts().ASTDumpAll = false;
        std::cout << "Dump AST successfully." << std::endl;
    }
```

可以看出，它包含创建子进程，调用相应的工具执行任务，所以包含 `Execute`阶段。

### 3-2

> 根据`include/driver.h`中`_Act`类型，请简述`src/driver.cpp`中`FrontendCodeGen()`方法的功能。

如果不是执行手工构建的 IR Module，那么先使用`clang`内置的方法构建 LLVM IR。然后将 IR module 写出到指定的文件中。

## 实验设计

### phase1

根据 demo 文件夹下的 c 程序以及它生成的 .ll 文件，学习 LLVM IR 指令与 c代码的对应，手动编写相应的 .ll 文件

### phase2

根据 demo 中的 cpp 文件，学习相应的 SysYF IR 应用编程接口，以及与 IR 和 c 代码的对应关系，写出剩余五个 cpp 文件。

先写出包含读入待排序整型数组的函数、划分函数、交换函数、快排函数的快排c代码，根据已经写过的熟悉的 SysYF IR 应用编程接口调整相应的 c 代码，然后编写 `qsort_gen.cpp`.

### phase3

phase2 已经编写过利用 SysYF IR 应用编程接口的快排 IR 的生成文件，SysYF IR 应用编程接口与  LLVM IR API 十分相似，所以只需根据 LLVM 文档做部分修改，即可得到（虽然实际实现过程中遇到了许多困难）。

## 实验难点及解决方案

+ 在 phase1 中，很多 IR 的都是用的地址值，直观上感觉是操作数，实际是地址值，如果需要使用对应的操作数，需要先取出操作数的地址值。

+ 在 phase2 中，在编写如下代码对应的生成代码时，由于各种跳转指令叠加，导致漏掉了 `index++`，而编写好的代码调试非常麻烦（甚至一开始不知道怎么调试）。

  ```c++
  while(i <= right){
          if (arr[i] < arr[pivot]) {
              swap(i, index);
              index++;
          }
          i++;
      }
  ```

  后来的解决方案是利用 `putint` 实现了输出调试。

+ 第三关，与第二关相比，待排序数组改成了浮点类型，所以在比较两数组元素大小时，需要改成浮点数比大小。这个本身并不是什么困难的点，但是在编写过程中，我错误地将所有的比较大小都改成了浮点数比较，包括数组元素下标之间。这是一个粗心导致的错误，非常低级，但是它足够隐秘，而且报错信息是段错误，导致我花了差不多四个小时来找到这个问题。最后是依靠 ` -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address` 一点一点找到了出错点。

## 实验总结

做完以后回顾，本次难度倒也没有那么大，或者说，它的思想其实挺直观，尤其助教老师给出了非常有帮助的 demo.

但是一个问题的存在大大提升了本次实验的任务量，那就是调试困难，所以寻找 BUG 的过程差不多占了本次实验的 40% 的时间，而且令人很有挫败感。

还有一个小问题，是各种类型和变量分布在不同的文件中，不过根据助教老师的提示，配置了代码补全后，这个问题也就解决了。

总体来说，这个实验给我的收获还是挺多的，它承接了之前的词法语法语义分析，让我加深了对课堂上理论知识的理解。

## 实验反馈

建议出一份详细的如何配置 llvm 代码补全的教程。
