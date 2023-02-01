#include "Pass.h"
#include "Constant.h"
#include "Instruction.h"
#include "StrengthReduction.h"
#include "DominateTree.h"
#include <cmath>

// std::map<std::string, Var_Tri> vat_map;
// 存储变量之间的线性关系， pair 的两个元素依次是乘法因子和加法因子，string 是

int check(int x){  // 判断一个数是不是 2 的次幂，O(1)
    if(x==0) return -1;
    int cnt=0;
    while(x%2==0){
        x >>= 1;
        cnt++;
    }
    if(x==1) return cnt;
    return -1;
}

void StrengthReduction::execute() {
    for(auto f: module->get_functions()){
        if(f->get_basic_blocks().empty()) continue;   // 函数体为空
        std::map<BasicBlock*, std::set<BasicBlock*> > loop;  // 从自然循环的入口到自然循环的映射
        find_loops(f, loop);
        cope_loop(loop);
        // 用移位指令代替部分乘法和除法指令
        constant_sub(f);
        easy_sub(f);
    }
}

void StrengthReduction::easy_sub(Function* f){
    for(auto bb:f->get_basic_blocks()){
        // 替换基本块里的高代价指令
        std::vector<std::pair<std::list<Instruction*>::iterator, Instruction* > > inst_to_sub; // 保存替换的指令
        std::vector<Instruction*> inst_to_be_sub; // 保存要被替换的指令
        for(auto inst:bb->get_instructions()){
           if(inst->is_mul()){
                // 整型乘法
                auto op0 = inst->get_operand(0), op1 = inst->get_operand(1);
                auto op0_check = dynamic_cast<ConstantInt*>(op0);
                auto op1_check = dynamic_cast<ConstantInt*>(op1);
                // 检查两个变量是不是常量
                if(!op0_check && !op1_check) 
                    continue;  // 两个都是变量时无法优化
                int op0_cnt = -1, op1_cnt = -1;
                // 检查两个变量是不是 2 的次幂
                if(op0_check){  // op0 是常量x
                    op0_cnt = check(op0_check->get_value());
                }
                if(op1_check){  // op1 是常量
                    op1_cnt = check(op1_check->get_value());
                }

                // 指令替换
                if(op0_cnt != -1){
                    // 指令相当于把 op1 左移 op0_cnt 位
                    auto pos = bb->find_instruction(inst);
                    inst_to_be_sub.push_back(inst);
                    auto sub_inst = BinaryInst::create_shl(op1, ConstantInt::get(op0_cnt, module), bb, module);
                    bb->delete_instr(sub_inst); // 前一行在创建时会把指令添加到 bb 最后面，这里把它删掉
                    op1->add_use(sub_inst, 0);
                    inst_to_sub.push_back(std::make_pair(pos, sub_inst));
                    inst->replace_all_use_with(sub_inst);
                }
                else if(op1_cnt != -1){
                    // 指令相当于把 op0 左移 op1_cnt 位
                    auto pos = bb->find_instruction(inst);
                    inst_to_be_sub.push_back(inst);
                    auto sub_inst = BinaryInst::create_shl(op0, ConstantInt::get(op1_cnt, module), bb, module);
                    bb->delete_instr(sub_inst); // 前一行在创建时会把指令添加到 bb 最后面，这里把它删掉
                    op0->add_use(sub_inst, 0);
                    inst_to_sub.push_back(std::make_pair(pos, sub_inst));
                    inst->replace_all_use_with(sub_inst);
                }
           }
           else if(inst->is_div()) {
                // 整型除法
                auto op0 = inst->get_operand(0), op1 = inst->get_operand(1);
                auto op1_check = dynamic_cast<ConstantInt*>(op1);   // 除数
                // 检查两个变量是不是常量
                if(!op1_check) 
                    continue;  // 被除数都是变量时无法优化
                int op1_cnt = -1;
                
                if(op1_check){  // op1 是常量
                    op1_cnt = check(op1_check->get_value());
                }

                // 指令替换
                if(op1_cnt != -1){
                    // 指令相当于把 op0 右移 op1_cnt 位
                    auto pos = bb->find_instruction(inst);
                    inst_to_be_sub.push_back(inst);
                    auto sub_inst = BinaryInst::create_ashr(op0, ConstantInt::get(op1_cnt, module), bb, module);
                    bb->delete_instr(sub_inst); // 前一行在创建时会把指令添加到 bb 最后面，这里把它删掉
                    op0->add_use(sub_inst, 0);
                    inst_to_sub.push_back(std::make_pair(pos, sub_inst));
                    inst->replace_all_use_with(sub_inst);
                }
           } 
        }
        int len = inst_to_sub.size();
        for(int i=0; i<len; i++){
            bb->add_instruction(inst_to_sub[i].first, inst_to_sub[i].second);
            // 所有引用到 inst_to_be_sub[i] 的地方全都替换成 inst_to_sub[i].second
            bb->delete_instr(inst_to_be_sub[i]);
            // 替换指令的同时要更新引用链
        }
    }
}

void StrengthReduction::reverse_dfs(BasicBlock* bb, std::set<BasicBlock*>& visited){
    // 反向 dfs
    visited.insert(bb);
    auto children = bb->get_pre_basic_blocks();
    for(auto child:children){
        // 遍历孩子节点
        if(visited.find(child) != visited.end()) continue;  // 访问过了
        reverse_dfs(child, visited);
    }
}

void StrengthReduction::cal_dom_set(Function* f){
    auto entry_bb = f->get_entry_block();
    for(auto bb:f->get_basic_blocks()){
        dom[bb].insert(bb);
        dom[bb].insert(entry_bb);
        auto temp = bb->get_idom();
        while(temp != entry_bb){
            dom[bb].insert(temp);
            temp = temp->get_idom();
        }
    }
}

bool StrengthReduction::is_dom(BasicBlock* bb1, BasicBlock* bb2){
    if(dom[bb1].find(bb2) == dom[bb1].end()) return false;
    return true;
}

void StrengthReduction::find_loops(Function* f, std::map<BasicBlock*, std::set<BasicBlock*> > &loop){
    cal_dom_set(f); // 计算每个基本块的支配集合
    for(auto bb:f->get_basic_blocks()){
        for(auto succ:bb->get_succ_basic_blocks()){
            // 寻找回边
            if(is_dom(bb, succ)){
                // succ 是 bb 的支配节点
                std::set<BasicBlock*> loop_visited;
                loop_visited.insert(succ);  // 将 succ 设置为已访问，进行反向 dfs，找出循环中的全部节点
                reverse_dfs(bb, loop_visited);
                if(loop.find(succ) != loop.end() && loop_visited.size() < loop[succ].size()){
                    continue;   
                    // 对于一个基本块是多个循环的入口的情况，保留最大的那个循环（continue 出现的情况）
                }
                loop[succ] = loop_visited;
            }
        }
    }
}

void StrengthReduction::find_inductive_var(std::map<Value*, Var_Tri> &var, BasicBlock* entry, std::map<BasicBlock*, std::set<BasicBlock*> > &loop){
    auto loop_elm = loop[entry];
    // 第一次遍历，寻找基本循环变量
    for(auto inst:entry->get_instructions()){
        // 找出所有的 phi 指令
        if(inst->is_phi()){
            // basic_inv.push_back(inst);
            // 只有所有分支来源都是自增或者自减或常量的情况才能优化
            int num = inst->get_num_operand();
            bool check=true;
            for(int i=0; i<num; i+=2){
                Instruction* ori_v = dynamic_cast<Instruction*>(inst->get_operand(i));    // 来自哪个值
                BasicBlock* ori_bb = dynamic_cast<BasicBlock *>(inst->get_operand(i+1));    // 相应的基本块
                if(!ori_v || loop[entry].find(ori_bb) == loop[entry].end()) continue;   
                // 说明是常量或者来源的基本块不是本循环内的基本块，跳过
                if(ori_v->is_add() || ori_v->is_sub()){
                    auto op0 = ori_v->get_operand(0), op1 = ori_v->get_operand(1);
                    if(!((op0 == inst && dynamic_cast<ConstantInt*>(op1)) || \
                    (op1 == inst && dynamic_cast<ConstantInt*>(op0)))){
                        // 不是自增或自减，则不是循环变量
                        check = false;
                        break;
                    }
                }
                else{
                    check = false;
                    break;
                }
            }
            if(check){
                var[inst] = Var_Tri(inst, 1, 0);
            }
        }
    }
    // 没有基本循环变量，则返回
    if(var.empty()) return ;

    // 不断遍历，寻找线性依赖于循环变量的变量，直到收敛
    bool changed = true;
    while(changed){
        changed = false;
        for(auto bb:loop_elm){  // 遍历循环中的基本块
            for(auto inst:bb->get_instructions()){
                if(var.find(inst) != var.end()) continue;
                // 如果指令已经在 var 中出现过，说明其已经被标记为归纳变量，无需再次标记
                if(inst->is_mul()){
                    auto op0 = inst->get_operand(0), op1 = inst->get_operand(1);
                    if(dynamic_cast<ConstantInt*>(op0) && !dynamic_cast<ConstantInt*>(op1)){
                        if(var.find(op1) != var.end()){
                            auto bas = var[op1].val_;   // 找出对应的基本循环变量 
                            auto op0_val = dynamic_cast<ConstantInt*>(op0);
                            var[inst] = Var_Tri(bas, var[op1].mul*op0_val->get_value(), var[op1].add*op0_val->get_value());
                            // 添加新的对应关系 
                            changed = true;
                        }
                    }
                    else if(dynamic_cast<ConstantInt*>(op1) && !dynamic_cast<ConstantInt*>(op0)){
                        if(var.find(op0) != var.end()){
                            auto bas = var[op0].val_;   // 找出对应的基本循环变量
                            auto op1_val = dynamic_cast<ConstantInt*>(op1);
                            var[inst] = Var_Tri(bas, var[op0].mul*op1_val->get_value(), var[op0].add*op1_val->get_value());
                            // 添加新的对应关系 
                            changed = true;
                        }
                    }
                }

                else if(inst->is_add()){
                    auto op0 = inst->get_operand(0), op1 = inst->get_operand(1);
                    if(dynamic_cast<ConstantInt*>(op0) && !dynamic_cast<ConstantInt*>(op1)){
                        if(var.find(op1) != var.end()){
                            auto bas = var[op1].val_;   // 找出对应的基本循环变量 
                            auto op0_val = dynamic_cast<ConstantInt*>(op0);
                            var[inst] = Var_Tri(bas, var[op1].mul, var[op1].add+op0_val->get_value());
                            // 添加新的对应关系 
                            changed = true;
                        }
                    }
                    else if(dynamic_cast<ConstantInt*>(op1) && !dynamic_cast<ConstantInt*>(op0)){
                        if(var.find(op0) != var.end()){
                            auto bas = var[op0].val_;   // 找出对应的基本循环变量
                            auto op1_val = dynamic_cast<ConstantInt*>(op1);
                            var[inst] = Var_Tri(bas, var[op0].mul, var[op0].add+op1_val->get_value());
                            // 添加新的对应关系 
                            changed = true;
                        }
                    }
                }

                else if(inst->is_sub()){
                    auto op0 = inst->get_operand(0), op1 = inst->get_operand(1);
                    if(dynamic_cast<ConstantInt*>(op0) && !dynamic_cast<ConstantInt*>(op1)){
                        // 形式为 num-i
                        if(var.find(op1) != var.end()){
                            auto bas = var[op1].val_;   // 找出对应的基本循环变量 
                            auto op0_val = dynamic_cast<ConstantInt*>(op0);
                            var[inst] = Var_Tri(bas, -var[op1].mul, op0_val->get_value()-var[op1].add);
                            // 添加新的对应关系 
                            changed = true;
                        }
                    }
                    else if(dynamic_cast<ConstantInt*>(op1) && !dynamic_cast<ConstantInt*>(op0)){
                        // 形式为 i-num
                        if(var.find(op0) != var.end()){
                            auto bas = var[op0].val_;   // 找出对应的基本循环变量
                            auto op1_val = dynamic_cast<ConstantInt*>(op1);
                            var[inst] = Var_Tri(bas, var[op0].mul, var[op0].add-op1_val->get_value());
                            // 添加新的对应关系 
                            changed = true;
                        }
                    }
                }
                
            }
        }
    }
}

void StrengthReduction::cope_loop(std::map<BasicBlock*, std::set<BasicBlock*> > &loop){
    // 处理每个循环
    // 找出循环变量和与循环变量相关的变量
    // 完成归纳变量的强度削弱
    for(auto loop_elm: loop){
        BasicBlock* entry = loop_elm.first; // 入口基本块
        // std::vector<Value *> basic_inv; // 基本循环变量
        std::map<Value*, Var_Tri> var;   // 将变量映射到三元组
        
        find_inductive_var(var, entry, loop);
        // 找出所有的归纳变量

        for(auto var_elm: var){
            // 遍历所有的归纳变量
            Instruction* inst = dynamic_cast<Instruction*>(var_elm.first);
            Var_Tri var_tri = var_elm.second;
            if(inst->is_mul() && var_tri.mul > 1){// && inst->get_parent()==entry){
                // 该指令是乘法指令，且位于入口块，有优化的空间
                // 下一步是用 phi 指令代替原指令
                // 在 ori 的每一个源基本块上还要加入相应的自增指令
                Instruction* basic = dynamic_cast<Instruction*>(var_tri.val_);
                // basic 是和 var_elm 直接相关的循环变量
                int num = basic->get_num_operand();
                // basic 是一条 phi 指令，其每一对来源或是来自循环外的基本块，
                // 或是循环内的基本块，且相应的变量是当前变量的自增、自减或常量
                PhiInst* new_phi = PhiInst::create_phi(basic->get_type(), entry);
                entry->delete_instr(new_phi);
                auto ins_pos_ = entry->find_instruction(basic);
                ins_pos_++;
                entry->add_instruction(ins_pos_, new_phi);
                // 新的 phi 指令的类型和循环变量相同
                for(int i = 0; i < num; i+=2){
                    auto ori_op = basic->get_operand(i);    // 来源操作数
                    BasicBlock* ori_bb = dynamic_cast<BasicBlock*>(basic->get_operand(i+1));  // 来源基本块
                    auto try_const = dynamic_cast<ConstantInt*>(ori_op);
                    if(try_const){  // ori_op 是常量，则直接处理
                        new_phi->add_phi_pair_operand(ConstantInt::get(try_const->get_value()*var_tri.mul+var_tri.add, module), ori_bb);
                        continue;
                    }
                    
                    // 用新的 phi 指令替换 inst
                    inst->replace_all_use_with(new_phi);
                    BasicBlock* inst_bb = inst->get_parent();
                    inst_bb->delete_instr(inst);
                    
                    // 以下的情况都是 ori_op 是变量的情况
                    if(loop[entry].find(ori_bb) == loop[entry].end()){
                        // ori_bb 不在循环里，则是进入循环前的初始化操作

                        auto end_pos = ori_bb->find_instruction(ori_bb->get_terminator());
                        end_pos--;
                        end_pos--;
                        // 在前一基本块的最后一条指令之前插入初始化的指令
                        auto init_inst = BinaryInst::create_mul(ConstantInt::get(var_tri.mul, module), ori_op, ori_bb, module);
                        ori_bb->delete_instr(init_inst);
                        ori_op->add_use(init_inst, 1);
                        ori_bb->add_instruction(end_pos, init_inst);

                        auto init_inst_add = BinaryInst::create_add(ConstantInt::get(var_tri.add, module), init_inst, ori_bb, module);
                        ori_bb->delete_instr(init_inst_add);
                        init_inst->add_use(init_inst_add, 1);
                        end_pos = ori_bb->find_instruction(ori_bb->get_terminator());
                        end_pos--;
                        end_pos--;
                        ori_bb->add_instruction(end_pos, init_inst_add);

                        new_phi->add_phi_pair_operand(init_inst_add, ori_bb);
                    }
                    else{
                        // ori_bb 在循环里，则 ori_op 是对循环变量的自增或自减运算
                        Instruction* ori_inst = dynamic_cast<Instruction*>(ori_op);
                        BasicBlock* temp_bb = ori_inst->get_parent();
                        // 不确定 ori_inst 是不是一定在 ori_bb 里

                        // 这个 ori_inst 一定是循环变量的加法或者减法运算指令
                        // ori_inst 必然一个操作数是常量，另一个操作数时循环变量
                        auto ins_pos = temp_bb->find_instruction(ori_inst);
                        ins_pos++;
                        auto op0 = dynamic_cast<ConstantInt*>(ori_inst->get_operand(0));
                        auto op1 = dynamic_cast<ConstantInt*>(ori_inst->get_operand(1));
                        if(ori_inst->is_add()){
                            ConstantInt* temp;
                            if(op0) temp = op0;
                            else temp = op1;
                            auto sub_inst = BinaryInst::create_add(new_phi, ConstantInt::get(var_tri.mul*temp->get_value(), module), ori_bb, module);
                            // ori_inst->replace_all_use_with(sub_inst);
                            ori_bb->delete_instr(sub_inst);
                            new_phi->add_use(sub_inst, 0);
                            // 恢复引用链
                            ori_bb->add_instruction(ins_pos, sub_inst);
                            new_phi->add_phi_pair_operand(sub_inst, ori_bb);
                        }
                        else if(ori_inst->is_sub()){
                            ConstantInt* temp;
                            if(op0) temp = op0;
                            else temp = op1;
                            auto sub_inst = BinaryInst::create_sub(new_phi, ConstantInt::get(var_tri.mul*temp->get_value(), module), ori_bb, module);
                            // ori_inst->replace_all_use_with(sub_inst);
                            ori_bb->delete_instr(sub_inst);
                            new_phi->add_use(sub_inst,0);
                            // 恢复引用链
                            ori_bb->add_instruction(ins_pos, sub_inst);
                            new_phi->add_phi_pair_operand(sub_inst, ori_bb);
                        }
                    }
                }
            }
        }
    }
}

void StrengthReduction::constant_sub(Function *f){
    for(auto bb:f->get_basic_blocks()){
        std::vector<Instruction*> inst_to_del;  // 保存待删除的指令
        for(auto inst:bb->get_instructions()){
            if(inst->is_mul()){
                auto op0_try = dynamic_cast<ConstantInt*>(inst->get_operand(0));
                auto op1_try = dynamic_cast<ConstantInt*>(inst->get_operand(1));
                if((op0_try&&op0_try->get_value()==0) || (op1_try&&op1_try->get_value()==0)){
                    // 乘法结果必然是 0，这条指令可以直接删除
                    inst->replace_all_use_with(ConstantInt::get(0, module));
                    inst_to_del.push_back(inst);
                }
                else if(op0_try && op0_try->get_value()==1){
                    // 乘 1，表明是复写，可以直接替换
                    inst->replace_all_use_with(inst->get_operand(1));
                    inst_to_del.push_back(inst);
                }
                else if(op1_try && op1_try->get_value()==1){
                    inst->replace_all_use_with(inst->get_operand(0));
                    inst_to_del.push_back(inst);
                }
            }
            else if(inst->is_div()){
                auto op1_try = dynamic_cast<ConstantInt*>(inst->get_operand(1));
                if(op1_try && op1_try->get_value()==1){
                    inst->replace_all_use_with(inst->get_operand(0));
                    inst_to_del.push_back(inst);
                }
            }
            else if(inst->is_add()){
                auto op0_try = dynamic_cast<ConstantInt*>(inst->get_operand(0));
                auto op1_try = dynamic_cast<ConstantInt*>(inst->get_operand(1));
                if(op0_try && op0_try->get_value()==0){
                    // +0 也是复写
                    inst->replace_all_use_with(inst->get_operand(1));
                    inst_to_del.push_back(inst);
                }
                else if(op1_try && op1_try->get_value()==0){
                    // +0 也是复写
                    inst->replace_all_use_with(inst->get_operand(0));
                    inst_to_del.push_back(inst);
                }
            }
            else if(inst->is_sub()){
                auto op1_try = dynamic_cast<ConstantInt*>(inst->get_operand(1));
                if(op1_try && op1_try->get_value()==0){
                    // -0 也是复写
                    inst->replace_all_use_with(inst->get_operand(0));
                    inst_to_del.push_back(inst);
                }
                else if(inst->get_operand(0) == inst->get_operand(1)){
                    // a-a 的情况
                    inst->replace_all_use_with(ConstantInt::get(0, module));
                    inst_to_del.push_back(inst);
                }
            }
        }
        for(auto inst:inst_to_del){
            bb->delete_instr(inst);
        }
    }
}
