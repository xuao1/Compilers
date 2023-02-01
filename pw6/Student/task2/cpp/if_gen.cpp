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
    auto module = new Module("if_test code");
    auto builder = new IRStmtBuilder(nullptr, module);
    Type* Int32Type = Type::get_int32_type(module);
    Type* FloatType = Type::get_float_type(module);

    // ȫ������ int a
    auto zero_initializer = ConstantZero::get(Int32Type, module);
    auto a = GlobalVariable::create("a", module, Int32Type, false, zero_initializer);

    // main ����
    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
        "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    // BasicBlock������������������ν,���ǿ��Է����Ķ�
    builder->set_insert_point(bb); // һ��BB�Ŀ�ʼ,����ǰ����ָ����λ������bb

    auto retAlloca = builder->create_alloca(Int32Type); // ���ڴ��з��䷵��ֵ��λ��

    auto retBB = BasicBlock::create(
        module, "", mainFun);  // return��֧,��ǰcreate,�Ա�true��֧����br

    // a = 10;
    builder->create_store(CONST_INT(10), a);
    
    /*
    if( a>0 ){
		return a;
	}
	return 0;
    */
    auto aLoad = builder->create_load(a);
    auto icmp = builder->create_icmp_gt(aLoad, CONST_INT(0));  // a��0�ıȽ�

    auto trueBB = BasicBlock::create(module, "trueBB_if", mainFun);    // true��֧
    auto falseBB = BasicBlock::create(module, "falseBB_if", mainFun);  // false��֧

    builder->create_cond_br(icmp, trueBB, falseBB);  // ����BR
    builder->set_insert_point(trueBB);  // if true; ��֧�Ŀ�ʼ��ҪSetInsertPoint����
    aLoad = builder->create_load(a);
    builder->create_store(aLoad, retAlloca);
    builder->create_br(retBB);  // br retBB

    builder->set_insert_point(falseBB);  // if false
    builder->create_br(retBB);  // br retBB
    
    builder->set_insert_point(retBB);  // ret��֧
    auto retLoad = builder->create_load(retAlloca);
    builder->create_ret(retLoad);

    std::cout << module->print();
    delete module;
    return 0;
}