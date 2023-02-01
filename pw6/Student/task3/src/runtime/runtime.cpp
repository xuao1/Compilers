#include <iostream>
#include "runtime/io.h"
#include "runtime/runtime.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

using namespace std;
using namespace llvm;

runtime_info::runtime_info(Module* module)
{
    // 参考 /include/SysYFIRBuilder/IRBuilder.h
    auto TyVoid = Type::getVoidTy(module->getContext());
    auto TyInt32 = Type::getInt32Ty(module->getContext());
    auto TyInt32Ptr = Type::getInt32PtrTy(module->getContext());
    auto TyFloat = Type::getFloatTy(module->getContext());
    auto TyFloatPtr = Type::getFloatPtrTy(module->getContext());

    auto input_type = FunctionType::get(TyInt32, {}, false);
    get_int_func = 
        Function::Create(
            input_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "getint", // IR中的函数命名，保持与输入输出函数命名一致即可
            module); 
    
    input_type = FunctionType::get(TyFloat, {}, false);
    get_float_func =
        Function::Create(
            input_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "getfloat", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);

    input_type = FunctionType::get(TyInt32, {}, false);
    get_char_func =
        Function::Create(
            input_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "getch", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);

    std::vector<Type *> input_params;
    std::vector<Type *>().swap(input_params);
    input_params.push_back(TyInt32Ptr);
    input_type = FunctionType::get(TyInt32, input_params, false);
    get_int_array_func =
        Function::Create(
            input_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "getarray", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);
    
    std::vector<Type*>().swap(input_params);
    input_params.push_back(TyFloatPtr);
    input_type = FunctionType::get(TyInt32, input_params, false);
    get_float_array_func =
        Function::Create(
            input_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "getfarray", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);

    std::vector<Type*> output_params;
    std::vector<Type*>().swap(output_params);
    output_params.push_back(TyInt32);
    auto output_type = FunctionType::get(TyVoid, output_params, false);
    put_int_func =
        Function::Create(
            output_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "putint", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);

    std::vector<Type*>().swap(output_params);
    output_params.push_back(TyFloat);
    auto output_type = FunctionType::get(TyVoid, output_params, false);
    put_float_func =
        Function::Create(
            output_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "putfloat", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);

    std::vector<Type*>().swap(output_params);
    output_params.push_back(TyInt32);
    auto output_type = FunctionType::get(TyVoid, output_params, false);
    put_char_func =
        Function::Create(
            output_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "putch", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);

    std::vector<Type*>().swap(output_params);
    output_params.push_back(TyInt32);
    output_params.push_back(TyInt32Ptr);
    auto output_type = FunctionType::get(TyVoid, output_params, false);
    put_int_array_func =
        Function::Create(
            output_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "putarray", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);

    std::vector<Type*>().swap(output_params);
    output_params.push_back(TyInt32);
    output_params.push_back(TyFloatPtr);
    auto output_type = FunctionType::get(TyVoid, output_params, false);
    put_float_array_func =
        Function::Create(
            output_type,
            GlobalValue::LinkageTypes::ExternalLinkage,
            "putfarray", // IR中的函数命名，保持与输入输出函数命名一致即可
            module);
}

using namespace string_literals;


std::vector<std::tuple<std::string, void *>> runtime_info::get_runtime_symbols()
{
    return {
        make_tuple("getint"s, (void *)&::getint),
        // TODO
         };
}