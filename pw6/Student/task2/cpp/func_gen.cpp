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
    auto module = new Module("func_test code");
    auto builder = new IRStmtBuilder(nullptr, module);
    Type* Int32Type = Type::get_int32_type(module);
    Type* FloatType = Type::get_float_type(module);

    /* add 函数
    int add(int a, int b) {
        return (a + b - 1);
    }
    */
    std::vector<Type*> Ints(2, Int32Type); // 函数参数类型的vector
    auto addFunTy = FunctionType::get(Int32Type, Ints); //通过返回值类型与参数类型列表得到函数类型
    auto addFun = Function::create(addFunTy,"add", module); // 由函数类型得到函数

    // BB的名字在生成中无所谓,但是可以方便阅读
    auto bb = BasicBlock::create(module, "entry", addFun);
    builder->set_insert_point(bb);                        // 一个BB的开始,将当前插入指令点的位置设在bb

    auto retAlloca = builder->create_alloca(Int32Type);   // 在内存中分配返回值的位置
    auto aAlloca = builder->create_alloca(Int32Type);     // 在内存中分配参数a的位置
    auto bAlloca = builder->create_alloca(Int32Type);     // 在内存中分配参数b的位置

    
    std::vector<Value*> args;  // 获取add函数的形参,通过Function中的iterator
    for (auto arg = addFun->arg_begin(); arg != addFun->arg_end(); arg++) {
        args.push_back(*arg);   // * 号运算符是从迭代器中取出迭代器当前指向的元素
    }

    builder->create_store(args[0], aAlloca);  // store参数a
    builder->create_store(args[1], bAlloca);  // store参数a
    auto aLoad = builder->create_load(aAlloca);
    auto bLoad = builder->create_load(bAlloca);

    // a+b-1
    auto add = builder->create_iadd(aLoad, bLoad);
    auto sub = builder->create_isub(add, CONST_INT(1));

    builder->create_store(sub, retAlloca);
    auto retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);


    // main 函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
        "main", module);
    bb = BasicBlock::create(module, "entry", mainFun);
    // BasicBlock的名字在生成中无所谓,但是可以方便阅读
    builder->set_insert_point(bb); // 一个BB的开始,将当前插入指令点的位置设在bb

    retAlloca = builder->create_alloca(Int32Type); // 在内存中分配返回值的位置

    // int a=3,b=2,c=5
    aAlloca = builder->create_alloca(Int32Type);
    bAlloca = builder->create_alloca(Int32Type);
    auto cAlloca = builder->create_alloca(Int32Type);

    builder->create_store(CONST_INT(3), aAlloca);
    builder->create_store(CONST_INT(2), bAlloca);
    builder->create_store(CONST_INT(5), cAlloca);

    // return c + add(a,b);
    aLoad = builder->create_load(aAlloca);
    bLoad = builder->create_load(bAlloca);
    auto cLoad = builder->create_load(cAlloca);
    auto add_result = builder->create_call(addFun, { aLoad, bLoad });
    add = builder->create_iadd(cLoad, add_result);

    builder->create_store(add, retAlloca);
    retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);

    std::cout << module->print();
    delete module;
    return 0;
}
