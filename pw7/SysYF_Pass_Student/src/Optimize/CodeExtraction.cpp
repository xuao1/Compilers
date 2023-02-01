#include "CodeExtraction.h"
#include "Instruction.h"
#include "StrengthReduction.h"

void CodeExtraction::execute(){
    for(auto f:module->get_functions()){
        std::map<BasicBlock*, std::set<BasicBlock*> > loop;
        StrengthReduction sr(module);
        sr.find_loops(f, loop); // 找出 f 中的循环
        cope_loop(loop);    // 
    }
}


BinaryInst* CodeExtraction::copy(Instruction* inst){
    auto bi = dynamic_cast<BinaryInst*>(inst);
    if(!bi){ return nullptr; }
    if(bi->is_add()){
        auto temp = BinaryInst::create_add(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);    
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    else if(bi->is_sub()){
        auto temp = BinaryInst::create_sub(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    else if(bi->is_mul()){
        auto temp = BinaryInst::create_mul(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    else if(bi->is_div()){
        auto temp = BinaryInst::create_sdiv(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    else if(bi->is_fadd()){
        auto temp = BinaryInst::create_fadd(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    else if(bi->is_fsub()){
        auto temp = BinaryInst::create_fsub(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    else if(bi->is_fmul()){
        auto temp = BinaryInst::create_fmul(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    else if(bi->is_fdiv()){
        auto temp = BinaryInst::create_fdiv(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    else if(bi->is_rem()){
        auto temp = BinaryInst::create_srem(bi->get_operand(0), bi->get_operand(1), bi->get_parent(), module);
        bi->get_parent()->delete_instr(temp);
        bi->get_operand(0)->add_use(temp, 0);
        bi->get_operand(1)->add_use(temp, 1);
        return temp;
    }
    return nullptr;
}

void CodeExtraction::cope_loop(std::map<BasicBlock*, std::set<BasicBlock*> > &loop){
    
    for(auto loop_elm:loop){
        BasicBlock* entry = loop_elm.first; // 获得循环入口
        std::set<BasicBlock*> loop_bb = loop_elm.second;
        std::set<Instruction*> invar;
        find_invar(loop_bb, invar); //寻找循环不变量
        std::vector<BasicBlock*> pre_bbs;
        for(auto pre_bb:entry->get_pre_basic_blocks()){
            if(loop_bb.find(pre_bb) != loop_bb.end()) 
                continue;
            pre_bbs.push_back(pre_bb);
        }

        for(auto var:invar){
           
            if(pre_bbs.size() == 1){
                auto pre_bb = pre_bbs[0];
                BasicBlock* par = var->get_parent();
                if(loop_bb.find(par) == loop_bb.end()) continue;
                // var 同时是两个循环（一大一小）里的不变量，之前已经有循环把它提出去过了

                if(var->is_binary()){
                    auto op0 = var->get_operand(0);
                    auto op1 = var->get_operand(1);
                    par->delete_instr(var);
                    auto ins_pos = pre_bb->find_instruction(pre_bb->get_terminator());
                    // ins_pos--; ins_pos--;
                    pre_bb->add_instruction(ins_pos, var);
                    op0->add_use(var,0);
                    op1->add_use(var,1);
                    // 恢复 use 链

                }
                // else if(var->is_call()){
                //     auto par_inst = par->get_instructions();
                //     // par_inst.remove(var);   // 只是在指令列表中被删除
                //     Function* ff = var->get_function();
                //     int num = var->get_num_operand();
                //     std::vector<Value*> args;
                //     for(int i=1; i<num; i++){
                //         args.push_back(var->get_operand(i));
                //     }
                //     par->delete_instr(var);
                //     auto ins_pos = pre_bb->find_instruction(pre_bb->get_terminator());
                //     pre_bb->add_instruction(ins_pos, var);
                //     ff->add_use(var, 0);
                //     for(int i=1; i<num; i++){
                //         args[i-1]->add_use(var, i);
                //     }
                //     // 恢复 use 链
                // }
                
            }
            else{
                if(var->is_binary()){
                    auto new_phi = PhiInst::create_phi(var->get_type(), entry);
                    entry->delete_instr(new_phi);
                    bool flag = true;
                    for(auto pre_bb:pre_bbs){
                        auto new_inst = copy(var);
                        if(new_inst == nullptr) // 出错则不进行替换
                        {
                            flag = false;
                            break;
                        }
                        auto ins_pos = pre_bb->find_instruction(pre_bb->get_terminator());
                        //ins_pos--; ins_pos--;
                        pre_bb->add_instruction(ins_pos, var);
                        new_phi->add_phi_pair_operand(new_inst, pre_bb);
                    }
                    if(!flag)   // 出错，则不对这个变量进行替换
                        continue;
                    var->replace_all_use_with(new_phi);
                    auto pos = entry->find_instruction(entry->get_instructions().front());
                    entry->add_instruction(pos, new_phi);
                    // 把 new_phi 插在 entry 的最前面
                    BasicBlock* par = var->get_parent();
                    par->delete_instr(var);
                }
            //     else if(var->is_call()){
            //         auto new_phi = PhiInst::create_phi(var->get_type(), entry);
            //         entry->delete_instr(new_phi);
            //         Function* ff = var->get_function();
            //         int num = var->get_num_operand();
            //         std::vector<Value*> args;
            //         for(int i=1; i<num; i++){
            //             args.push_back(var->get_operand(i));
            //         }
            //         for(auto pre_bb:pre_bbs){
            //             auto new_inst = CallInst::create(ff, args, pre_bb);
            //             auto pre_inst = pre_bb->get_instructions();
            //             pre_inst.remove(new_inst);
            //             auto ins_pos = pre_bb->find_instruction(pre_bb->get_terminator());
            //             //ins_pos--; ins_pos--;
            //             pre_bb->add_instruction(ins_pos, var);
            //             new_phi->add_phi_pair_operand(new_inst, pre_bb);
            //         }

            //         var->replace_all_use_with(new_phi);
            //         auto pos = entry->find_instruction(entry->get_instructions().front());
            //         entry->add_instruction(pos, new_phi);
            //         // 把 new_phi 插在 entry 的最前面
            //         BasicBlock* par = var->get_parent();
            //         par->delete_instr(var);
            //     } 
            }
        }
    }
}

void CodeExtraction::find_invar(std::set<BasicBlock*> &loop_bb, std::set<Instruction*> &invar){
    for(auto bb:loop_bb){
            // 遍历循环中的基本块
            for(auto inst:bb->get_instructions()){
                // 遍历基本块中的指令
                // if(inst->is_call()){
                //     int num = inst->get_num_operand();
                //     bool flag = false;
                //     for(int i=1; i<num; i++){
                //         Instruction* op = dynamic_cast<Instruction*>(inst->get_operand(i));
                //         if(!op) continue;
                //         BasicBlock* par = op->get_parent();
                //         if(loop_bb.find(par) != loop_bb.end()){
                //             // 在循环中被定值，则无法外提
                //             flag = true;
                //             break;
                //         }
                //     }
                //     if(!flag){
                //         // 该指令的参数都不是在循环中定值，则可以外提
                //         invar.insert(inst);
                //     }
                // }
                if(!inst->is_binary()) continue;
                // 不是双目运算指令则不会外提
                auto op0 = inst->get_operand(0);
                auto op1 = inst->get_operand(1);
                bool check0 = false, check1 = false;

                if(dynamic_cast<Constant*>(op0)){
                    check0 = true;
                }
                else{
                    // 若 op0 不是常量，则一定是变量，检查它被定值的基本块
                    Instruction* temp = dynamic_cast<Instruction*>(op0);
                    if(!temp) continue;
                    BasicBlock* par = temp->get_parent();
                    if(loop_bb.find(par) == loop_bb.end()){
                        // temp 不在循环中定值，则可以提到循环外面
                        check0 = true;
                    }
                }

                if(dynamic_cast<Constant*>(op1)){
                    check1 = true;
                }
                else{
                    // 若 op0 不是常量，则一定是变量，检查它被定值的基本块
                    Instruction* temp = dynamic_cast<Instruction*>(op1);
                    if(!temp) continue;
                    BasicBlock* par = temp->get_parent();
                    if(loop_bb.find(par) == loop_bb.end()){
                        // temp 不在循环中定值，则可以提到循环外面
                        check1 = true;
                    }
                }

                if(check0 && check1){
                    invar.insert(inst);
                }
            }
        }
}

