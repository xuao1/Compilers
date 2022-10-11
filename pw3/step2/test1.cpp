#include<iostream>
#include<memory>
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
unique_ptr<Test> fun()
{
    return unique_ptr<Test>(new Test("789"));
}
int main()
{
    shared_ptr<Test> ptest(new Test("123"));	//调用构造函数输出 Test create
    weak_ptr<Test> ptest2(ptest);               //根据 share_ptr 创建 weak_ptr
    ptest.reset();                              //释放该 share_ptr
    shared_ptr<Test> ptest3 = ptest2.lock();
    assert(ptest3.get() == NULL);               //判断不能访问
    cout << "cannot" << endl;
    return 0;
}