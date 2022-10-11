## 问题

你认为`unique_ptr`可以作为实参传递给函数吗，为什么？请将你的思考放在`src/step1/`文件夹下，并且命名为`answer.md`。

> Hint: 从值传递和[引用传递](https://en.wikipedia.org/wiki/Evaluation_strategy#Call_by_reference)两方面考虑。

## 回答

参考这个链接[M.6 — std::unique_ptr – Learn C++ (learncpp.com)](https://www.learncpp.com/cpp-tutorial/stdunique_ptr/)

由以下这段描述：

> If you want the function to take ownership of the contents of the pointer, pass the std::unique_ptr by value. Note that because copy semantics have been disabled, you’ll need to use std::move to actually pass the variable in.
>
> Note that in this case, ownership of the Resource was transferred to takeOwnership(), so the Resource was destroyed at the end of takeOwnership() rather than the end of main().
>
> However, most of the time, you won’t want the function to take ownership of the resource. Although you can pass a std::unique_ptr by reference (which will allow the function to use the object without assuming ownership), you should only do so when the called function might alter or change the object being managed.

可知：

直接复制的值传递，即 copy assignment，这种传递参数的方式对于 `unique_ptr` 是不被允许的。

但是可以用 `move` 的方式，将指所有权转移给函数，但是这样做存在的一个问题是，当子函数结束后，这个对象也就被删除了。

而引用传递是可以的，子函数可以引用该指针，而且子函数结束后，我们仍可以使用该对象。