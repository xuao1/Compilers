#include "Check.h"
#include "Module.h"
#include <algorithm>

void Check::execute() {
    //TODO write your IR Module checker here.
    for(auto f:module->get_functions()){
        if(f->get_basic_blocks().empty()){
            continue;
        }
        
        if(!(err_type&1) && !check_end(f)){
            err_type |= 1;
        }

        if(!(err_type&2) && !check_ops(f)){
            err_type |= 2;
        }

        // if(!(err_type&4) && !check_pre(f)){
        //     // 检查 phi 中的前驱是否正确
        //     err_type |= 4;
        // }

        if(!(err_type&8) && !check_bb(f)){
            err_type |= 8;
        }

        if(!(err_type&16) && !check_def_use(f)){
            err_type |= 16;
        }
    }
    if(err_type){   // 中间表示有问题
        std::cout << "err type" << err_type << std::endl;
        print();
        exit(0);    // 直接退出
    }
}

bool Check::check_end(Function *f) {
    for(auto bb:f->get_basic_blocks()){
        auto terminate_inst = bb->get_terminator(); // 获得最后一条指令
        if(!terminate_inst) return false;   // 说明最后一条指令不是终结指令
    }
    return true;
}

bool Check::check_ops(Function *f) {
    // 检查操作数在引用前是否定值
    std::map<BasicBlock*, std::set<Value*> > bb_def; // 用于记录每个标准块内定值的变量（要求所有标准块都有名字）
    
    auto gvs = module->get_global_variable();    // 获得全局变量
    std::map<Value*, bool> is_gv; // 记录有定值的变量
    
    for(auto gv:gvs){
        is_gv[gv] = true;   // 全局变量都是初始化过的
    }

    for(auto bb:f->get_basic_blocks()){
        // 记录每个基本块定义的变量
        //std::string bb_name = bb->get_name();
        bb_def[bb] = {};
        for(auto inst:bb->get_instructions()){

            if(inst->is_store()){
                auto dest = inst->get_operand(1);
                bb_def[bb].insert(dest);
            }
            else if(!inst->is_void()){
                // auto dest = inst->get_name();
                bb_def[bb].insert(inst);
            }
        }
    }

    bool changed = true;
    std::map<BasicBlock*, std::set<Value*> > in_bb, out_bb;
    for(auto bb:f->get_basic_blocks()){ //初始化
        in_bb[bb] = {};
        out_bb[bb] = bb_def[bb];
    }

    // 利用数据流分析求每个基本块入口处一定有定义的变量集合
    // 即无论按照哪条路径执行，这个基本块的入口处该变量总有定义
    int cnt = 0;
    while(changed ){
        cnt++;
        // std::cout << "Cycle: " << cnt << std::endl;
        changed = false;
        for(auto bb:f->get_basic_blocks()){
            // std::cout << bb->get_name() << std::endl;
            // std::cout << in_bb[bb->get_name()].size() << std::endl;
            // std::string bb_name = bb->get_name();
            std::set<Value*> inter; // 存储所有前驱 out 的交集
            inter = {};
            bool flag = false;
            for(auto pre_bb:bb->get_pre_basic_blocks()){
                // 对所有前驱的 out 取交集
                // std::string pre_name = pre_bb->get_name();
                // std::cout<<"\t"<<pre_name<<std::endl;
                if(!flag){  // 表明是第一次合并
                    inter = out_bb[pre_bb];
                    flag = true;
                }
                else{
                    std::set_intersection(out_bb[pre_bb].begin(), out_bb[pre_bb].end(), inter.begin(), inter.end(), \
                                            std::inserter(inter, inter.begin()));
                }
            }

            std::map<std::set<Value*>, bool> temp_m;
            temp_m[inter] = true;
            // 检查  in_bb 是否发生变化
            // std::cout<< "len_inter: " << inter.size() << std::endl;
            // std::cout<< "len_in: " << in_bb[bb_name].size() << std::endl;
            if(temp_m.find(in_bb[bb]) == temp_m.end()){
                // 两个集合不相等
                in_bb[bb] = inter;
                changed = true;
                // outB = inB ∪ defB
            }
            std::set_union(in_bb[bb].begin(), in_bb[bb].end(), bb_def[bb].begin(), bb_def[bb].end(), \
                                std::inserter(out_bb[bb], out_bb[bb].begin()));
            // std::cout<<std::endl;
        }
    }
    
    for(auto arg:f->get_args()){
        is_gv[arg] = true; // 把函数的参数也看作全局变量
    }

    for(auto bb:f->get_basic_blocks()){
        std::map<Value*, bool> temp_m;
        std::string bb_name = bb->get_name();
        
        for(auto inst:bb->get_instructions()){
            if(inst->is_store()){
                auto dest = inst->get_operand(1);
                temp_m[dest] = true;
                // std::cout << dest << std::endl;
                // std::cout << inst->print() << std::endl;
            }
            else if(!inst->is_void()){
                std::string dest = inst->get_name();
                if(dest=="") continue;  // ret 语句如果返回值非空也不会有名字
                // std::cout << dest << std::endl;
                // std::cout << inst->print() << std::endl;
                if(temp_m.find(inst)!=temp_m.end() && temp_m[inst]){
                    // 重复定值，报错
                    std::cout << "Redefinition of " << dest << std::endl;
                    return false;
                }
                temp_m[inst] = true;
            }

            // 检查每一次变量引用前该变量是否一定有定值 --------
            if(inst->is_binary() || inst->is_float_binary() || inst->is_cmp() || inst->is_fcmp()){
            // 有两个操作数
                for(int i=0; i<2; i++){   // 遍历参数
                    auto op = inst->get_operand(i);
                    if(is_gv[op] || dynamic_cast<Constant*>(inst->get_operand(i))) 
                        continue;   
                    // 全局变量或常量
                    if(!temp_m[op] && in_bb[bb].find(op)==in_bb[bb].end()){
                        std::cout << inst->print() << std::endl;
                        return false;
                        // 如果没在当前基本块定义，且可能没有在入口处定值，则产生错误
                    }
                }
            }
            else if(inst->is_store() || inst->is_load() || (inst->is_ret() && !inst->is_void()) || \
                    (inst->is_br() && inst->get_num_operand()==3) || inst->is_zext() \
                    || inst->is_fptosi() || inst->is_sitofp()){
            // 只检查第一个操作数
                auto src = inst->get_operand(0);
                if(is_gv[src] || dynamic_cast<Constant*>(inst->get_operand(0))) 
                    continue;
                if(!temp_m[src] && in_bb[bb].find(src)==in_bb[bb].end()){
                        std::cout << inst->print() << std::endl;
                        return false;
                }
            }
            else if(inst->is_call()){   // 检查 call 的参数列表
            // 检查第一个操作数以外的操作数
                int n = inst->get_num_operand();
                for(int i=1; i<n; i++){   // 遍历参数
                    auto arg = inst->get_operand(i);
                    if(is_gv[arg] || dynamic_cast<Constant*>(inst->get_operand(i)))   // 是全局变量或常量
                        continue;
                    if(!temp_m[arg] && in_bb[bb].find(arg)==in_bb[bb].end()){
                        std::cout << inst->print() << std::endl;
                        return false;
                    }
                }
            }
            else if(inst->is_phi()){
                int n = inst->get_num_operand();
                for(int i=0; i<n; i++){ // 只有编号为偶数的操作数才是值引用
                    if(i%2==0){
                        auto op = inst->get_operand(i);
                        if(is_gv[op] || dynamic_cast<Constant*>(inst->get_operand(i)))   // 是全局变量或常量
                            continue;
                        auto ori_bb = dynamic_cast<BasicBlock*>(inst->get_operand(i+1));   // 获得相应的 bb
                        if(!temp_m[op] && in_bb[ori_bb].find(op)==in_bb[ori_bb].end() && \
                            out_bb[ori_bb].find(op)==out_bb[ori_bb].end()){
                            std::cout << inst->print() << std::endl;
                            // 这个变量不在当前基本块定义，也不在相应的前驱定义
                            return false;
                        }
                    }
                }
            }
            else if(inst->is_gep()){
                // 检查所有操作数
                int n = inst->get_num_operand();
                for(int i=0; i<n; i++){   // 遍历参数
                    auto ind = inst->get_operand(i);
                    if(is_gv[ind] || dynamic_cast<Constant*>(inst->get_operand(i)))   // 是全局变量或常量
                        continue;
                    if(!temp_m[ind] && in_bb[bb].find(ind)==in_bb[bb].end()){
                        std::cout << inst->print() << std::endl;
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool Check::check_pre(Function *f){
    // 这段代码已被弃用
    for(auto bb:f->get_basic_blocks()){
        std::set<std::string> pre_names;    // 前驱名字的并集
        
        for(auto pre_bb:bb->get_pre_basic_blocks()){
            pre_names.insert(pre_bb->get_name());
        }

        for(auto inst:bb->get_instructions()){
            if(inst->is_phi()){
                int num = inst->get_num_operand();
                for(int i=1; i<num; i+=2){  // 遍历基本块参数
                    if(pre_names.find(inst->get_operand(i)->get_name()) == pre_names.end()){
                        // 这个基本块不是当前基本块的前驱
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool Check::check_bb(Function *f){
    std::map<BasicBlock *, std::set<BasicBlock *> > m_suc;
    std::map<BasicBlock *, std::set<BasicBlock *> > m_pre;
    // 集合的查找效率比链表更高
    // m_suc[bb] 里存了 bb 全部的后继节点
    for(auto bb:f->get_basic_blocks()){
        m_suc[bb] = {};
        m_pre[bb] = {};
        for(auto suc_bb:bb->get_succ_basic_blocks()){
            m_suc[bb].insert(suc_bb);
        }
        for(auto pre_bb:bb->get_pre_basic_blocks()){
            m_pre[bb].insert(pre_bb);
        }
    }
    
    // 检查 bb 的前驱节点 pre_bb 的后继里是否有 bb
    for(auto bb:f->get_basic_blocks()){
        for(auto pre_bb:bb->get_pre_basic_blocks()){
            if(m_suc[pre_bb].find(bb) == m_suc[pre_bb].end())   // bb 应该在 pre_bb 的后继节点集合里
                return false;
        }
        for(auto suc_bb:bb->get_succ_basic_blocks()){
            if(m_pre[suc_bb].find(bb) == m_pre[suc_bb].end())   // bb 应该在 suc_bb 的前驱节点集合里
                return false;
        }
    }
    return true;
}

bool Check::check_def_use(Function* f){
    // 检查每一条指令的每一次引用是否对得上
    for(auto bb:f->get_basic_blocks()){
        for(auto inst:bb->get_instructions()){
            auto use_list = inst->get_use_list();
            // 检查这个变量引用链里的指令是不是都正确引用了它
            for(auto use_elm:use_list){
                Instruction* use_inst = dynamic_cast<Instruction*>(use_elm.val_);
                int use_no = use_elm.arg_no_;   // 第几个操作数
                if(use_inst->get_operand(use_no) != inst)
                    return false;
            }
            int num = inst->get_num_operand();
            for(int i=0; i<num; i++){
                // 检查该指令是不是在其（变量）操作数的 use 链里
                Instruction* temp = dynamic_cast<Instruction*>(inst->get_operand(i));
                if(!temp) continue;
                auto use_list_ = temp->get_use_list();
                bool flag = false;
                for(auto use_elm_:use_list_){
                    if(use_elm_.val_==inst && use_elm_.arg_no_==i){
                        flag = true;                        
                        break;
                    }
                }
                if(!flag)   // 没找到
                    return false;
            }  
        }
    }
    return true;
}

void Check::print(){
    if(err_type&1){
        std::cout << "Basicblock terminator check error." << std::endl;
    }
    if(err_type&2){
        std::cout << "Operands check error. Uninitialized operand(s) found." << std::endl;
    }
    if(err_type&4){
        std::cout << "Phi check error." << std::endl;
    }
    if(err_type&8){
        std::cout << "Basicblock topological relation check error." << std::endl;
    }
    if(err_type&16){
        std::cout << "Def use chain check error." << std::endl;
    }
}