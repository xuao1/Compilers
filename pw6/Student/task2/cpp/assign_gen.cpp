#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRStmtBuilder.h"
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
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFloat::get(num, module) // �õ�����ֵ�ı�ʾ,����������õ�

int main(){
    auto module = new Module("assign_test code");  
    auto builder = new IRStmtBuilder(nullptr, module);
    Type* Int32Type = Type::get_int32_type(module);
    Type* FloatType = Type::get_float_type(module);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
        "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    // BasicBlock������������������ν,���ǿ��Է����Ķ�
    builder->set_insert_point(bb); // һ��BB�Ŀ�ʼ,����ǰ����ָ����λ������bb

    auto retAlloca = builder->create_alloca(Int32Type); // ���ڴ��з��䷵��ֵ��λ��
    

    // float b=1.8
    auto bAlloca = builder->create_alloca(FloatType);
    builder->create_store(CONST_FP(1.8), bAlloca);

    // int a[2] = {2};
    auto* arrayType_a = ArrayType::get(Int32Type, 2);
    auto aAlloca = builder->create_alloca(arrayType_a);

    auto a0Gep = builder->create_gep(aAlloca, { CONST_INT(0), CONST_INT(0) });
    builder->create_store(CONST_INT(2), a0Gep);

    // a[1] = a[0] * b;
    a0Gep = builder->create_gep(aAlloca, { CONST_INT(0), CONST_INT(0) });
    auto a0Load = builder->create_load(a0Gep);
    auto a0toFloat = builder->create_sitofp(a0Load, FloatType); // (float)a[0]

    auto bLoad = builder->create_load(bAlloca);
    auto mulresult = builder->create_fmul(a0toFloat, bLoad); // a[0]*b

    auto mul2int = builder->create_fptosi(mulresult, Int32Type); // (int)(a[0]*b)

    auto a1Gep = builder->create_gep(aAlloca, { CONST_INT(0), CONST_INT(1) });
    builder->create_store(mul2int, a1Gep);

    // return a[1]
    a1Gep = builder->create_gep(aAlloca, { CONST_INT(0), CONST_INT(1) });
    auto a1Load = builder->create_load(a1Gep);
    builder->create_store(a1Load, retAlloca);
    auto retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);
    
    std::cout << module->print();
    delete module;
    return 0;
}
