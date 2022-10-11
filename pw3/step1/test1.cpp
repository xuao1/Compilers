#include<iostream>
#include<memory>	//auto_ptr的头文件
#include<cassert>
using namespace std;
class Test
{
public:
    Test(string s)
    {
        str = s;
        cout << "Test creat\n";
    }
    ~Test()
    {
        cout << "Test delete:" << str << endl;
    }
    string& getStr()
    {
        return str;
    }
    void setStr(string s)
    {
        str = s;
    }
    void print()
    {
        cout << str << endl;
    }
private:
    string str;
};

void func(auto_ptr<Test> ptest) {
    return;
}

int main()
{
    auto_ptr<Test> ptest(new Test("123"));	// 调用构造函数输出 Test creat
    ptest->print();							// 输出 123
    func(ptest);                            // "将 auto_ptr 按值传递给函数将导致您的资源被转移至函数参数，并在函数末尾被销毁"
    // 该函数退出后，ptest 对应的资源也被销毁了，会输出 "Test delete:123"
    assert(!ptest.get());                   // 判断 ptest 是否是空指针
    printf("ptest is null\n");              // 是空指针，输出
    return 0;								
}