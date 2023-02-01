#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRStmtBuilder.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // ���ڵ�����Ϣ,��ҿ����ڱ��������ͨ��" -DDEBUG"��������һѡ��
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // ����кŵļ�ʾ��
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module_ptr)

#define CONST_FP(num) \
    ConstantFloat::get(num, module_ptr) // �õ�����ֵ�ı�ʾ,����������õ�

int main() {
    auto builder = new IRBuilder();
    auto module = builder->getModule();
    auto module_ptr = module.get();
    auto scope = builder->getScope();
    auto stmt_builder = builder->getStmtBuilder();


    auto getint = scope->find("getint", true);
    auto putint = scope->find("putint", true);
    auto getfarray = scope->find("getfarray", true);
    auto putfarray = scope->find("putfarray", true);

    Type* Int32Type = Type::get_int32_type(module_ptr);
    Type* FloatType = Type::get_float_type(module_ptr);
    
    // main ����
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
        "main", module_ptr);
    auto bb = BasicBlock::create(module_ptr, "entry", mainFun);
    stmt_builder->set_insert_point(bb); // һ��BB�Ŀ�ʼ,����ǰ����ָ����λ������bb
    auto retAlloca = stmt_builder->create_alloca(Int32Type); // ���ڴ��з��䷵��ֵ��λ��

    // int a = getint();
    auto aAlloca = stmt_builder->create_alloca(Int32Type);
    /*
    auto getint =
            Function::create(
                    input_type,
                    "getint",
                    module.get());
    */
    auto aLoad = stmt_builder->create_call(getint, {});
    stmt_builder->create_store(aLoad, aAlloca);
    // putint(a);
    stmt_builder->create_call(putint, {aLoad});

    // float b[10000] = {1, 2};
    auto* arrayType_b = ArrayType::get(FloatType, 10000);
    auto bAlloca = stmt_builder->create_alloca(arrayType_b);

    auto b0Gep = stmt_builder->create_gep(bAlloca, { CONST_INT(0), CONST_INT(0) });
    stmt_builder->create_store(CONST_FP(1.0), b0Gep);

    auto b1Gep = stmt_builder->create_gep(bAlloca, { CONST_INT(0), CONST_INT(1) });
    stmt_builder->create_store(CONST_FP(2.0), b1Gep);

    // int n = getfarray(b);
    b0Gep = stmt_builder->create_gep(bAlloca, { CONST_INT(0), CONST_INT(0) });
    auto nAlloca = stmt_builder->create_alloca(Int32Type);
    auto nLoad = stmt_builder->create_call(getfarray, { b0Gep });
    stmt_builder->create_store(nLoad, nAlloca);

    // putfarray(n+1, b);
    // ���ߴ���ʵ���� b[0] �ĵ�ֵַ
    // nLoad = stmt_builder->create_load(nAlloca);
    b0Gep = stmt_builder->create_gep(bAlloca, { CONST_INT(0), CONST_INT(0) });
    auto add = stmt_builder->create_iadd(nLoad, CONST_INT(1));
    stmt_builder->create_call(putfarray, {add, b0Gep});

    // return b[0]
    b0Gep = stmt_builder->create_gep(bAlloca, { CONST_INT(0), CONST_INT(0) });
    auto b0Load = stmt_builder->create_load(b0Gep);

    // retLoad = (int)b[0]
    auto intb0Load = stmt_builder->create_fptosi(b0Load, Int32Type);
    stmt_builder->create_store(intb0Load, retAlloca);
    auto retLoad = stmt_builder->create_load(retAlloca);
    stmt_builder->create_ret(retLoad);

    std::cout << module_ptr->print();
    delete builder;
}