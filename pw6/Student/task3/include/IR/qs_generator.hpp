#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <iostream>
#include <memory>
#include "runtime/runtime.h"

using namespace llvm; // 指明命名空间为llvm

#define CONST(num) \
    ConstantInt::get(context, APInt(32, num)) //得到常数值的表示,方便后面多次用到

llvm::Module *qs_generator(LLVMContext &context, runtime_info *&runtime)
{
    IRBuilder<> builder(context);
    llvm::Type *Int32Type = llvm::Type::getInt32Ty(context);
    llvm::Type *FloatType = llvm::Type::getFloatTy(context);
    // IRBuilder<> builder(context);
    auto module = new llvm::Module("quickSort", context);  // module name是什么无关紧要
    
    runtime = new runtime_info(module);

    // float arr[4096]
	llvm::ArrayType* array_type = llvm::ArrayType::get(llvm::Type::getFloatTy(context), 4096);
	auto zero_init = llvm::ConstantAggregateZero::get(Int32Type);
    llvm::GlobalVariable* arr = new llvm::GlobalVariable(/*Module=*/*module,
		/*Type=*/array_type,
		/*isConstant=*/false,
		/*Linkage=*/llvm::GlobalValue::ExternalLinkage,
		/*Initializer=*/0, // has initializer, specified below
		/*Name=*/"arr");
	arr->setAlignment(llvm::MaybeAlign(16));
    arr->setInitializer(zero_init);

    
    // swap(int i, int j)
    // 函数参数类型的vector
    std::vector<llvm::Type *> Ints(2, Int32Type);
    auto swapFun = Function::Create(llvm::FunctionType::get(Int32Type, Ints, false),
                                    GlobalValue::LinkageTypes::ExternalLinkage,
                                    "swap", module);

    // BB的名字在生成中无所谓,但是可以方便阅读
    auto bb = BasicBlock::Create(context, "entry", swapFun);
    builder.SetInsertPoint(bb);                     // 一个BB的开始
    auto retAlloca = builder.CreateAlloca(Int32Type);  // 返回值的空间分配
    auto iAlloca = builder.CreateAlloca(Int32Type);    // 参数i的空间分配
    auto jAlloca = builder.CreateAlloca(Int32Type);    // 参数j的空间分配

    std::vector<Value *> args;  //获取gcd函数的参数,通过iterator
    for (auto arg = swapFun->arg_begin(); arg != swapFun->arg_end(); arg++) {
        args.push_back(arg);
    }

    builder.CreateStore(args[0], iAlloca);  //将参数i store下来
    builder.CreateStore(args[1], jAlloca);  //将参数j store下来
    auto iLoad = builder.CreateLoad(iAlloca);
    auto jLoad = builder.CreateLoad(jAlloca);

    // float temp = arr[i]
    auto tempAlloca = builder.CreateAlloca(FloatType);
    auto arriGep = builder.CreateGEP(arr,{CONST(0),iLoad});
    auto arriLoad = builder.CreateLoad(arriGep);
    builder.CreateStore(arriLoad, tempAlloca);
    auto tempLoad = builder.CreateLoad(tempAlloca);

    // arr[i] = arr[j]
    auto arrjGep = builder.CreateGEP(arr, {CONST(0), jLoad});
    auto arrjLoad = builder.CreateLoad(arrjGep);
    builder.CreateStore(arrjLoad, arriGep);

    // arr[j]=temp
    builder.CreateStore(tempLoad, arrjGep);

    // return
    builder.CreateRet(CONST(0));


    // partition(int left, int right)
    std::vector<llvm::Type *> Ints1(2, Int32Type);
    auto partFun = Function::Create(llvm::FunctionType::get(Int32Type, Ints1, false),
                                    GlobalValue::LinkageTypes::ExternalLinkage,
                                    "partition", module);

    // BB的名字在生成中无所谓,但是可以方便阅读
    bb = BasicBlock::Create(context, "entry", partFun);
    builder.SetInsertPoint(bb);                     // 一个BB的开始
    retAlloca = builder.CreateAlloca(Int32Type);  // 返回值的空间分配
    auto leftAlloca = builder.CreateAlloca(Int32Type);    // 参数i的空间分配
    auto rightAlloca = builder.CreateAlloca(Int32Type);    // 参数j的空间分配

    std::vector<Value *> args1;  //获取gcd函数的参数,通过iterator
    for (auto arg = partFun->arg_begin(); arg != partFun->arg_end(); arg++) {
        args1.push_back(arg);
    }

    builder.CreateStore(args1[0], leftAlloca);  //将参数i store下来
    builder.CreateStore(args1[1], rightAlloca);  //将参数j store下来
    auto leftLoad = builder.CreateLoad(leftAlloca);
    auto rightLoad = builder.CreateLoad(rightAlloca);

    // int pivot = left
    auto pivotAlloca = builder.CreateAlloca(Int32Type);
    builder.CreateStore(leftLoad, pivotAlloca);
    auto pivotLoad = builder.CreateLoad(pivotAlloca);

    // int index = pivot+1
    auto indexAlloca = builder.CreateAlloca(Int32Type);
    auto pivotadd = builder.CreateAdd(pivotLoad, CONST(1));
    builder.CreateStore(pivotadd,indexAlloca);

    // int i = index
    iAlloca = builder.CreateAlloca(Int32Type);
    auto indexLoad = builder.CreateLoad(indexAlloca);
    builder.CreateStore(indexLoad, iAlloca);
    iLoad = builder.CreateLoad(iAlloca);

    // while
    auto condBB = BasicBlock::Create(context, "condBB_while", partFun);
    auto trueBB = BasicBlock::Create(context, "trueBB_while", partFun);
    auto falseBB = BasicBlock::Create(context, "falseBB_while", partFun);

    builder.CreateBr(condBB);
    builder.SetInsertPoint(condBB);
    // while(i<=right)
    iLoad = builder.CreateLoad(iAlloca);
    rightLoad = builder.CreateLoad(rightAlloca);
    auto icmp = builder.CreateICmpSLE(iLoad, rightLoad);

    builder.CreateCondBr(icmp, trueBB, falseBB);

    // true
    builder.SetInsertPoint(trueBB);
    // if (arr[i] < arr[pivot])
    iLoad = builder.CreateLoad(iAlloca);
    arriGep = builder.CreateGEP(arr,{CONST(0),iLoad});
    arriLoad = builder.CreateLoad(arriGep);
    pivotLoad = builder.CreateLoad(pivotAlloca);
    auto arripivotGep = builder.CreateGEP(arr,{CONST(0), pivotLoad});
    auto arrpivotLoad = builder.CreateLoad(arripivotGep);
    auto icmpif = builder.CreateFCmpOLT(arriLoad, arrpivotLoad);
    auto trueBBif = BasicBlock::Create(context, "trueBB_if", partFun);
    auto falseBBif = BasicBlock::Create(context, "falseBB_if", partFun);
    builder.CreateCondBr(icmpif, trueBBif, falseBBif);

    // true if
    builder.SetInsertPoint(trueBBif);
    // swap(i, index)
    iLoad = builder.CreateLoad(iAlloca);
    indexLoad = builder.CreateLoad(indexAlloca);
    builder.CreateCall(swapFun, {iLoad, indexLoad});
    // index++
    indexLoad = builder.CreateLoad(indexAlloca);
    auto newindex = builder.CreateAdd(indexLoad, CONST(1));
    builder.CreateStore(newindex, indexAlloca);
    // i++
    auto newi = builder.CreateAdd(iLoad, CONST(1));
    builder.CreateStore(newi, iAlloca);
    builder.CreateBr(condBB);

    // flase if
    builder.SetInsertPoint(falseBBif);
    iLoad = builder.CreateLoad(iAlloca);
    newi = builder.CreateAdd(iLoad, CONST(1));
    builder.CreateStore(newi, iAlloca);
    builder.CreateBr(condBB);

    // while false
    builder.SetInsertPoint(falseBB);
    // swap(pivot, index - 1)
    pivotLoad = builder.CreateLoad(pivotAlloca);
    indexLoad = builder.CreateLoad(indexAlloca);
    auto indexsub = builder.CreateSub(indexLoad, CONST(1));
    builder.CreateCall(swapFun, {pivotLoad, indexsub});
    // return index-1
    indexLoad = builder.CreateLoad(indexAlloca);
    indexsub = builder.CreateSub(indexLoad, CONST(1));
    builder.CreateStore(indexsub, retAlloca);
    retLoad = builder.CreateLoad(retAlloca);
    builder.CreateRet(retLoad);


    // quickSort(int left, int right)
    std::vector<llvm::Type *> Ints2(2, Int32Type);
    auto quickFun = Function::Create(llvm::FunctionType::get(Int32Type, Ints2, false),
                                    GlobalValue::LinkageTypes::ExternalLinkage,
                                    "quickSort", module);

    // BB的名字在生成中无所谓,但是可以方便阅读
    bb = BasicBlock::Create(context, "entry", quickFun);
    builder.SetInsertPoint(bb);                     // 一个BB的开始
    retAlloca = builder.CreateAlloca(Int32Type);  // 返回值的空间分配
    leftAlloca = builder.CreateAlloca(Int32Type);    // 参数i的空间分配
    rightAlloca = builder.CreateAlloca(Int32Type);    // 参数j的空间分配

    std::vector<Value *> args2;  //获取gcd函数的参数,通过iterator
    for (auto arg = quickFun->arg_begin(); arg != quickFun->arg_end(); arg++) {
        args2.push_back(arg);
    }

    builder.CreateStore(args2[0], leftAlloca);  //将参数i store下来
    builder.CreateStore(args2[1], rightAlloca);  //将参数j store下来
    leftLoad = builder.CreateLoad(leftAlloca);
    rightLoad = builder.CreateLoad(rightAlloca);

    // if(left < right)
    icmpif = builder.CreateICmpSLT(leftLoad, rightLoad);
    trueBBif = BasicBlock::Create(context, "trueBB_if", quickFun);
    falseBBif = BasicBlock::Create(context, "falseBB_if", quickFun);
    builder.CreateCondBr(icmpif, trueBBif, falseBBif);

    // if true
    builder.SetInsertPoint(trueBBif);
    // int partIndex = partition(left, right);
    auto partIndexAlloca = builder.CreateAlloca(Int32Type);
    auto partIndexLoad = builder.CreateCall(partFun, {leftLoad, rightLoad});
    builder.CreateStore(partIndexLoad, partIndexAlloca);
    // quickSort(left, partIndex - 1)
    auto partIndexsub = builder.CreateSub(partIndexLoad, CONST(1));
    builder.CreateCall(quickFun, {leftLoad, partIndexsub});
    // qucikSort(partitionIndex + 1, right)
    auto partIndexadd = builder.CreateAdd(partIndexLoad, CONST(1));
    builder.CreateCall(quickFun, {partIndexadd, rightLoad});
    builder.CreateRet(CONST(0));

    // if false
    builder.SetInsertPoint(falseBBif);
    builder.CreateRet(CONST(0));



    // main
    auto mainFun = Function::Create(llvm::FunctionType::get(Int32Type, false),
                                    GlobalValue::LinkageTypes::ExternalLinkage,
                                    "main", module);
    bb = BasicBlock::Create(context, "entry", mainFun);
    // BasicBlock的名字在生成中无所谓,但是可以方便阅读
    builder.SetInsertPoint(bb);
    retAlloca = builder.CreateAlloca(Int32Type);

    // int n = getfarray(arr);
    auto nAlloca = builder.CreateAlloca(Int32Type);
    auto arr0Gep = builder.CreateGEP(arr, {CONST(0), CONST(0)});
    auto arr_n = builder.CreateCall(runtime->get_float_array_func, {arr0Gep});
    builder.CreateStore(arr_n, nAlloca);
    // int start = current_time();
    auto startAlloca = builder.CreateAlloca(Int32Type);
    auto startLoad = builder.CreateCall(runtime->current_time_func, {});
    builder.CreateStore(startLoad, startAlloca);
    // quickSort(0,n-1)
    auto nLoad = builder.CreateLoad(nAlloca);
    auto nsub = builder.CreateSub(nLoad, CONST(1));
    builder.CreateCall(quickFun, {CONST(0), nsub});
    // int end = current_time();
    auto endAlloca = builder.CreateAlloca(Int32Type);
    auto endLoad = builder.CreateCall(runtime->current_time_func, {});
    builder.CreateStore(endLoad, endAlloca);
    // putfarray(n, arr);
    builder.CreateCall(runtime->put_float_array_func, {nLoad, arr0Gep});
    // putint(end - start);
    startLoad = builder.CreateLoad(startAlloca);
    endLoad = builder.CreateLoad(endAlloca);
    auto realtime = builder.CreateSub(endLoad, startLoad);
    builder.CreateCall(runtime->put_int_func, {realtime});
    // cout<<" ms\n";
    builder.CreateCall(runtime->put_char_func, {' '});
    builder.CreateCall(runtime->put_char_func, {'m'});
    builder.CreateCall(runtime->put_char_func, {'s'});
    builder.CreateCall(runtime->put_char_func, {'\n'});
    builder.CreateStore(CONST(0),retAlloca);
    retLoad = builder.CreateLoad(retAlloca);
    builder.CreateRet(retLoad);

    builder.ClearInsertionPoint();
    return module;
}
