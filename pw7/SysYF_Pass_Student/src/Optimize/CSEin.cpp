#include "CSEin.h"
#include <algorithm>

void CSEin::execute() {
    module->set_print_name();
    // 每个基本块内的公共子表达式删除
    // 基本块内的代码是顺序执行，所以只需依次遍历各条指令
    // 用set记录各条表达式，对于当前表达式，如果之前出现过，则删除，否则加入set
    // 需要做：判断表达式相等，替换对应的变量，删除表达式
    std::vector<Instruction*> delete_I;
    for(auto &func : this->module->get_functions()){
        if(func->get_basic_blocks().empty()) continue;
        for(auto bb: func->get_basic_blocks()){
            ExprInsts.clear();
            delete_I.clear();
            for(auto inst: bb->get_instructions()){
                // 每个基本块，顺序遍历指令
                // 是表达式：算数指令
                bool FindFlag=false;
                if(inst->is_binary() || inst->is_float_binary()){
                    for(auto inst0: ExprInsts){
                        if(ExprEqual(inst, inst0)){
                            FindFlag=true;
                            // 删除、替换
                            inst->replace_all_use_with(inst0);
                            // bb->delete_instr(inst);
                            delete_I.push_back(inst);
                            break;
                        }
                    }
                    if(FindFlag==false){
                        ExprInsts.push_back(inst);
                    }
                }
            }
            for(auto &&inst: delete_I){
                bb->delete_instr(inst);
            }
        }
    }
}

bool CSEin::ExprEqual(Instruction *inst1, Instruction *inst2){
    if(inst1->get_instr_type() != inst2->get_instr_type()) // opcode
        return false;
    auto operands1 = inst1->get_operands();
    auto operands2 = inst2->get_operands();
    if(operands1 == operands2)
        return true;
    else{
        if(operands1.size() != operands2.size()) return false;
        for(int i=0;i<operands1.size();i++){
            auto constint1=dynamic_cast<ConstantInt *>(operands1[i]);
            auto constint2=dynamic_cast<ConstantInt *>(operands2[i]);
            auto constfloat1=dynamic_cast<ConstantFloat *>(operands1[i]);
            auto constfloat2=dynamic_cast<ConstantFloat *>(operands1[i]);
            if(constint1 && constint2 && constint1->get_value()!=constint2->get_value()) return false;
            else if(constfloat1 && constfloat2 && constfloat1->get_value()!= constfloat2->get_value()) return false;
            else if(operands1[i]!=operands2[i]) return false;
        }
        return true;
    }
}
