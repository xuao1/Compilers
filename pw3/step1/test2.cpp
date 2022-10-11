#include<iostream>
#include<memory>	//auto_ptr的头文件
#include<cassert>
#include <cstring>
using namespace std;

int main()
{
    // auto_ptr 的内部实现中，析构函数中删除对象使用 delete 而不是 delete[]
    // 所以 auto_ptr 不能用来管理数组指针，否则会引起内存泄漏
    // 如下例
    auto_ptr<int> ptest(new int[100]);
    memset(ptest.get(), 1, sizeof(int) * 100);  // 此时该数组内每个元素的值均为 16843009

    ptest.reset(new int); // 执行析构函数，只释放第一个元素
    return 0;
}