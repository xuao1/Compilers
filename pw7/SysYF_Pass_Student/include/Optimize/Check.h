#ifndef CHECK_H
#define CHECK_H

#include "BasicBlock.h"
#include "Function.h"
#include "Instruction.h"
#include "Module.h"
#include "Pass.h"

class Check: public Pass {  // 继承自 Pass 类，继承了属性 module（扫描的对象）
private:
    const std::string name = "Check";
    int err_type=0; // 错误类型（可以同时表示多种错误类型）
    // 第 1 位指示是否有基本块没有以 ret 或 br 结尾
public:
    explicit Check(Module *m): Pass(m) {}   // 构造方法和 Pass 相同
    ~Check() {}
    void execute() final;   // 执行扫描，final 关键字使子类无法重写该方法
    bool check_end(Function *f);    // 检查基本块是否以终结指令结尾
    bool check_ops(Function *f);    // 检查操作数在引用之前是否定值
    bool check_pre(Function *f);    // 检查 phi 指令中的 bb 是否是当前基本块的前驱
    bool check_bb(Function *f);     // 检查基本块的前驱后继关系是否正确
    bool check_def_use(Function *f); // 检查 def_use 链
    void print();   //输出报错信息
    const std::string get_name() const override {return name;}  // 返回改变扫描的名字
    // 这里的 override 表明 get_name 方法是基类方法的覆写
    // 加在最前面的 const 用于修饰返回值类型，希望调用者将返回值作为常量处理，在这里实际上可有可无
    // const 成员函数（const 加在函数头和函数体之间的成员函数）不能修改任何成员变量
    // 这里后面的 const 实际上是修饰隐含的 this 指针
    const int get_err_type() const {return err_type;} // 返回错误类型
};


#endif