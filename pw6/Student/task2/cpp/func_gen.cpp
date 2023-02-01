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

int main() {
    auto module = new Module("func_test code");
    auto builder = new IRStmtBuilder(nullptr, module);
    Type* Int32Type = Type::get_int32_type(module);
    Type* FloatType = Type::get_float_type(module);

    /* add ����
    int add(int a, int b) {
        return (a + b - 1);
    }
    */
    std::vector<Type*> Ints(2, Int32Type); // �����������͵�vector
    auto addFunTy = FunctionType::get(Int32Type, Ints); //ͨ������ֵ��������������б�õ���������
    auto addFun = Function::create(addFunTy,"add", module); // �ɺ������͵õ�����

    // BB������������������ν,���ǿ��Է����Ķ�
    auto bb = BasicBlock::create(module, "entry", addFun);
    builder->set_insert_point(bb);                        // һ��BB�Ŀ�ʼ,����ǰ����ָ����λ������bb

    auto retAlloca = builder->create_alloca(Int32Type);   // ���ڴ��з��䷵��ֵ��λ��
    auto aAlloca = builder->create_alloca(Int32Type);     // ���ڴ��з������a��λ��
    auto bAlloca = builder->create_alloca(Int32Type);     // ���ڴ��з������b��λ��

    
    std::vector<Value*> args;  // ��ȡadd�������β�,ͨ��Function�е�iterator
    for (auto arg = addFun->arg_begin(); arg != addFun->arg_end(); arg++) {
        args.push_back(*arg);   // * ��������Ǵӵ�������ȡ����������ǰָ���Ԫ��
    }

    builder->create_store(args[0], aAlloca);  // store����a
    builder->create_store(args[1], bAlloca);  // store����a
    auto aLoad = builder->create_load(aAlloca);
    auto bLoad = builder->create_load(bAlloca);

    // a+b-1
    auto add = builder->create_iadd(aLoad, bLoad);
    auto sub = builder->create_isub(add, CONST_INT(1));

    builder->create_store(sub, retAlloca);
    auto retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);


    // main ����
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
        "main", module);
    bb = BasicBlock::create(module, "entry", mainFun);
    // BasicBlock������������������ν,���ǿ��Է����Ķ�
    builder->set_insert_point(bb); // һ��BB�Ŀ�ʼ,����ǰ����ָ����λ������bb

    retAlloca = builder->create_alloca(Int32Type); // ���ڴ��з��䷵��ֵ��λ��

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
