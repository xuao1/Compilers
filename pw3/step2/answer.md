## 题目

给`weak_ptr`赋值(转化成`weak_ptr`)的`shared_ptr`称作管理该`weak_ptr`的`shared_ptr`。如果该`shared_ptr`释放了，那么还可以通过它原来管理的`weak_ptr`去访问对象吗？请在`src/step2/test1.cpp`文件中构造样例来找到上述问题的答案，并将你的答案和对样例的分析写在`src/step2/answer.md`中。

## 回答

经测试，不可以访问

样例首先创建了一个 `shared_ptr` 对象，然后使用该 `shared_ptr` 创建 `weak_ptr`，然后使用 `reset` 把该 `shared_ptr` 释放。由于不能通过`weak_ptr`直接访问对象的方法，应该先把它转化为`shared_ptr`，使用 `lock` 完成转化。之后经判断，转化后的 `shared_ptr` 指向了 NULL，所以不可以访问。
