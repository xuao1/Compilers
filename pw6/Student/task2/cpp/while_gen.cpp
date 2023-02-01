#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRStmtBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFloat::get(num, module) // 得到常数值的表示,方便后面多次用到

int main() {
    auto module = new Module("while_test code");
    auto builder = new IRStmtBuilder(nullptr, module);
    Type* Int32Type = Type::get_int32_type(module);
    Type* FloatType = Type::get_float_type(module);

    // 全部变量 int a; int b
    auto zero_initializer = ConstantZero::get(Int32Type, module);
    auto a = GlobalVariable::create("a", module, Int32Type, false, zero_initializer);
    auto b = GlobalVariable::create("b", module, Int32Type, false, zero_initializer);


    // main 函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
        "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    // BasicBlock的名字在生成中无所谓,但是可以方便阅读
    builder->set_insert_point(bb); // 一个BB的开始,将当前插入指令点的位置设在bb

    auto retAlloca = builder->create_alloca(Int32Type); // 在内存中分配返回值的位置

    // b=0;a=3
    builder->create_store(CONST_INT(0), b);
    builder->create_store(CONST_INT(3), a);

    /*
    while(a>0){	
		b = b+a;
		a = a-1;
	}
    */
    auto condBB = BasicBlock::create(module, "condBB_while", mainFun);  // 条件BB
    auto trueBB = BasicBlock::create(module, "trueBB_while", mainFun);    // true分支
    auto falseBB = BasicBlock::create(module, "falseBB_while", mainFun);  // false分支

    builder->create_br(condBB);
    builder->set_insert_point(condBB);
    auto aLoad = builder->create_load(a);
    auto icmp = builder->create_icmp_gt(aLoad, CONST_INT(0));
    builder->create_cond_br(icmp, trueBB, falseBB);

    // true
    builder->set_insert_point(trueBB);
    // b=b+a
    aLoad = builder->create_load(a);
    auto bLoad = builder->create_load(b);
    auto new_b = builder->create_iadd(aLoad, bLoad);
    builder->create_store(new_b, b);
    // a=a-1
    aLoad = builder->create_load(a);
    auto new_a = builder->create_isub(aLoad, CONST_INT(1));
    builder->create_store(new_a, a);
    // 回到条件判断语句
    builder->create_br(condBB);

    // false
    // return b;
    builder->set_insert_point(falseBB);
    bLoad = builder->create_load(b);
    builder->create_store(bLoad, retAlloca);
    auto retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);

    std::cout << module->print();
    delete module;
    return 0;
}