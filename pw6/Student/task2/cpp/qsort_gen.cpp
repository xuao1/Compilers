#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRStmtBuilder.h"
#include "IRBuilder.h"
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
    auto builder0 = new IRBuilder();
    auto module0 = builder0->getModule();
    auto module = module0.get();
    auto scope = builder0->getScope();
    auto builder = builder0->getStmtBuilder();


    auto getint = scope->find("getint", true);
    auto putint = scope->find("putint", true);
    auto getarray = scope->find("getarray", true);
    auto putarray = scope->find("putarray", true);

    Type* Int32Type = Type::get_int32_type(module);
    Type* FloatType = Type::get_float_type(module);

    // 全局变量n,数组arr[1000]
    auto* arrayType_arr = ArrayType::get(Int32Type, 1000);
    auto zero_initializer = ConstantZero::get(Int32Type, module);
    auto arr = GlobalVariable::create("arr", module, arrayType_arr, false, zero_initializer);// 参数解释：  名字name，所属module，全局变量类型type，
    auto n = GlobalVariable::create("n", module, Int32Type, false, zero_initializer);


    // swap 函数  int swap(int i, int j)
    std::vector<Type*> Ints(2, Int32Type); // 函数参数类型的vector
    auto swapFunTy = FunctionType::get(Int32Type, Ints); //通过返回值类型与参数类型列表得到函数类型
    auto swapFun = Function::create(swapFunTy, "swap", module); // 由函数类型得到函数
    auto bb = BasicBlock::create(module, "entry", swapFun);
    builder->set_insert_point(bb);

    auto retAlloca = builder->create_alloca(Int32Type);   // 在内存中分配返回值的位置
    auto iAlloca = builder->create_alloca(Int32Type);     // 在内存中分配参数i的位置
    auto jAlloca = builder->create_alloca(Int32Type);     // 在内存中分配参数j的位置

    std::vector<Value*> args;  // 获取swap函数的形参,通过Function中的iterator
    for (auto arg = swapFun->arg_begin(); arg != swapFun->arg_end(); arg++) {
        args.push_back(*arg);   // * 号运算符是从迭代器中取出迭代器当前指向的元素
    }
    builder->create_store(args[0], iAlloca);  // store参数i
    builder->create_store(args[1], jAlloca);  // store参数j
    auto iLoad = builder->create_load(iAlloca);
    auto jLoad = builder->create_load(jAlloca);

    // int temp = arr[i];
    auto tempAlloca = builder->create_alloca(Int32Type);
    auto arriGep = builder->create_gep(arr, { CONST_INT(0), iLoad });
    auto arriLoad = builder->create_load(arriGep);
    builder->create_store(arriLoad, tempAlloca);
    auto tempLoad = builder->create_load(tempAlloca);

    // arr[i] = arr[j];
    auto arrjGep = builder->create_gep(arr, { CONST_INT(0), jLoad });
    auto arrjLoad = builder->create_load(arrjGep);
    builder->create_store(arrjLoad, arriGep);

    // arr[j] = temp;
    builder->create_store(tempLoad, arrjGep);

    builder->create_ret(CONST_INT(0)); // return 


    // partition 函数
    // partition(int left, int right)
    std::vector<Type*> Ints1(2, Int32Type); // 函数参数类型的vector
    auto partFunTy = FunctionType::get(Int32Type, Ints1); //通过返回值类型与参数类型列表得到函数类型
    auto partFun = Function::create(partFunTy, "partition", module); // 由函数类型得到函数
    bb = BasicBlock::create(module, "entry", partFun);
    builder->set_insert_point(bb);

    retAlloca = builder->create_alloca(Int32Type);   // 在内存中分配返回值的位置
    auto leftAlloca = builder->create_alloca(Int32Type);     
    auto rightAlloca = builder->create_alloca(Int32Type);     

    std::vector<Value*> args1;  // 获取swap函数的形参,通过Function中的iterator
    for (auto arg = partFun->arg_begin(); arg != partFun->arg_end(); arg++) {
        args1.push_back(*arg);   // * 号运算符是从迭代器中取出迭代器当前指向的元素
    }
    builder->create_store(args1[0], leftAlloca);  // store参数left
    builder->create_store(args1[1], rightAlloca);  // store参数right
    auto leftLoad = builder->create_load(leftAlloca);
    auto rightLoad = builder->create_load(rightAlloca);

    // int pivot = left;
    auto pivotAlloca = builder->create_alloca(Int32Type);
    builder->create_store(leftLoad, pivotAlloca);
    auto pivotLoad = builder->create_load(pivotAlloca);
    
    // int index = pivot + 1;
    auto indexAlloca = builder->create_alloca(Int32Type);
    auto pivotadd = builder->create_iadd(pivotLoad, CONST_INT(1));
    builder->create_store(pivotadd, indexAlloca);

    // int i = index
    iAlloca = builder->create_alloca(Int32Type);
    auto indexLoad = builder->create_load(indexAlloca);
    builder->create_store(indexLoad, iAlloca);
    iLoad = builder->create_load(iAlloca);

    // while
    auto condBB = BasicBlock::create(module, "condBB_while", partFun);  // 条件BB
    auto trueBB = BasicBlock::create(module, "trueBB_while", partFun);    // true分支
    auto falseBB = BasicBlock::create(module, "falseBB_while", partFun);  // false分支

    builder->create_br(condBB);
    builder->set_insert_point(condBB);
    // while(i <= right)
    iLoad = builder->create_load(iAlloca);
    rightLoad = builder->create_load(rightAlloca);
    auto icmp = builder->create_icmp_le(iLoad, rightLoad);

    builder->create_cond_br(icmp, trueBB, falseBB);

    // true
    builder->set_insert_point(trueBB);
    //auto nLoad0 = builder->create_load(n);
    //auto arr0Gep0 = builder->create_gep(arr, { CONST_INT(0), CONST_INT(0) });
    //builder->create_call(putarray, { nLoad0, arr0Gep0 });

    // if 
    // if (arr[i] < arr[pivot])
    iLoad = builder->create_load(iAlloca);
    arriGep = builder->create_gep(arr, { CONST_INT(0), iLoad });
    arriLoad = builder->create_load(arriGep);
    pivotLoad = builder->create_load(pivotAlloca);
    auto arrpivotGep = builder->create_gep(arr, { CONST_INT(0), pivotLoad });
    auto arrpivotLoad = builder->create_load(arrpivotGep);
    auto icmpif = builder->create_icmp_lt(arriLoad, arrpivotLoad);
    auto trueBBif = BasicBlock::create(module, "trueBB_if", partFun);    // true分支
    auto falseBBif = BasicBlock::create(module, "falseBB_if", partFun);  // false分支
    builder->create_cond_br(icmpif, trueBBif, falseBBif);  // 条件BR

    // true_if
    builder->set_insert_point(trueBBif);  // if true; 分支的开始需要SetInsertPoint设置
    // swap(i, index);
    iLoad = builder->create_load(iAlloca);
    indexLoad = builder->create_load(indexAlloca);
    builder->create_call(swapFun, { iLoad, indexLoad });
    // index++
    indexLoad = builder->create_load(indexAlloca);
    auto newindex = builder->create_iadd(indexLoad, CONST_INT(1));
    builder->create_store(newindex, indexAlloca);
    // i++
    auto newi = builder->create_iadd(iLoad, CONST_INT(1));
    builder->create_store(newi, iAlloca);
    builder->create_br(condBB);

    // flase_if
    builder->set_insert_point(falseBBif);  // if false
    iLoad = builder->create_load(iAlloca);
    newi = builder->create_iadd(iLoad, CONST_INT(1));
    builder->create_store(newi, iAlloca);
    builder->create_br(condBB);

    // while 的 true 结束
    // while false
    builder->set_insert_point(falseBB);
    // swap(pivot, index - 1);
    pivotLoad = builder->create_load(pivotAlloca);
    indexLoad = builder->create_load(indexAlloca);
    auto indexsub = builder->create_isub(indexLoad, CONST_INT(1));
    builder->create_call(swapFun, { pivotLoad, indexsub });
    // return index-1
    indexLoad = builder->create_load(indexAlloca);
    indexsub = builder->create_isub(indexLoad, CONST_INT(1));
    builder->create_store(indexsub, retAlloca);
    auto retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);

    
    // quickSort(int left, int right)
    std::vector<Type*> Ints2(2, Int32Type); // 函数参数类型的vector
    auto quickFunTy = FunctionType::get(Int32Type, Ints2); //通过返回值类型与参数类型列表得到函数类型
    auto quickFun = Function::create(quickFunTy, "quickSort", module); // 由函数类型得到函数
    bb = BasicBlock::create(module, "entry", quickFun);
    builder->set_insert_point(bb);

    retAlloca = builder->create_alloca(Int32Type);   // 在内存中分配返回值的位置
    leftAlloca = builder->create_alloca(Int32Type);
    rightAlloca = builder->create_alloca(Int32Type);

    std::vector<Value*> args2;  // 获取quickSort函数的形参,通过Function中的iterator
    for (auto arg = quickFun->arg_begin(); arg != quickFun->arg_end(); arg++) {
        args2.push_back(*arg);   // * 号运算符是从迭代器中取出迭代器当前指向的元素
    }
    builder->create_store(args2[0], leftAlloca);  // store参数left
    builder->create_store(args2[1], rightAlloca);  // store参数right
    leftLoad = builder->create_load(leftAlloca);
    rightLoad = builder->create_load(rightAlloca);

    // if
    // if (left < right)
    icmpif = builder->create_icmp_lt(leftLoad, rightLoad);
    trueBBif = BasicBlock::create(module, "trueBB_if", quickFun);    // true分支
    falseBBif = BasicBlock::create(module, "falseBB_if", quickFun);  // false分支
    builder->create_cond_br(icmpif, trueBBif, falseBBif);  // 条件BR
    
    // true_if
    builder->set_insert_point(trueBBif);  // if true; 分支的开始需要SetInsertPoint设置
    // int partitionIndex = partition(left, right);
    auto partIndexAlloca = builder->create_alloca(Int32Type);
    auto partIndexLoad = builder->create_call(partFun, { leftLoad, rightLoad });
    builder->create_store(partIndexLoad, partIndexAlloca);
    // quickSort(left, partitionIndex - 1);
    auto partIndexsub = builder->create_isub(partIndexLoad, CONST_INT(1));
    builder->create_call(quickFun, { leftLoad, partIndexsub });
    // quickSort(partitionIndex + 1, right);
    auto partIndexadd = builder->create_iadd(partIndexLoad, CONST_INT(1));
    builder->create_call(quickFun, { partIndexadd, rightLoad });
    builder->create_ret(CONST_INT(0));

    // flase_if
    builder->set_insert_point(falseBBif);  // if false
    builder->create_ret(CONST_INT(0));



    // Read 函数
    // 通过返回值类型与参数类型列表得到函数类型
    auto ReadFunTy = FunctionType::get(Int32Type, {});
    // 由函数类型得到函数
    auto ReadFun = Function::create(ReadFunTy, "Read", module);
    bb = BasicBlock::create(module, "entry", ReadFun);
    builder->set_insert_point(bb);
    retAlloca = builder->create_alloca(Int32Type);

    // n = getint();
    auto getintn = builder->create_call(getint, {});
    builder->create_store(getintn, n);

    // getarray(arr);
    auto arr0Gep = builder->create_gep(arr, { CONST_INT(0), CONST_INT(0) });
    builder->create_call(getarray, { arr0Gep });

    builder->create_ret(CONST_INT(0)); 


    // main 函数
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
        "main", module);
    bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb); // 一个BB的开始,将当前插入指令点的位置设在bb
    retAlloca = builder->create_alloca(Int32Type); // 在内存中分配返回值的位置

    // Read()
    builder->create_call(ReadFun, {});
    // quickSort(0, n-1); 
    auto nLoad = builder->create_load(n);
    auto nsub = builder->create_isub(nLoad, CONST_INT(1));
    
    builder->create_call(quickFun, { CONST_INT(0), nsub });

    // putarray(n, arr);
    nLoad = builder->create_load(n);
    arr0Gep = builder->create_gep(arr, { CONST_INT(0), CONST_INT(0) });
    builder->create_call(putarray, { nLoad, arr0Gep });

    builder->create_store(CONST_INT(0), retAlloca);
    retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);

    std::cout << module->print();
    delete builder;
}