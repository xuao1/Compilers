#include "ActiveVar.h"
#include <fstream>
// #include <iostream>

#include <algorithm>

void ActiveVar::execute() {
    //  请不要修改该代码。在被评测时不要在中间的代码中重新调用set_print_name
    module->set_print_name();
    //

    for (auto &func : this->module->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        } else {
            func_ = func;  

            
            /*you need to finish this function*/
            // 思想：根据数据流方程更新，迭代直到不再发生改变
            // IN[B] = use_B ∪ (OUT[B]-def_B)
            // OUT[B] = ∪S是B的后继 { IN[S] ∪ S的PIN中来源于B的活跃}

            // 迭代前：
            // 首先计算每个 Block 的 use, def, PIN
            // 即每个块：没定义就使用的变量（不含phi指令），定义的变量，phi指令使用的变量
            // 用map记录当前func涉及到的所有块，每个块的use、def和PIN集合
            /*
            // 设置该BasicBlock入口处的活跃变量集合
            void set_live_in(std::set<Value*> in){live_in = in;}
            // 设置该BasicBlock出口处的活跃变量集合
            void set_live_out(std::set<Value*> out){live_out = out;}
            // 获取该BasicBlock入口处的活跃变量集合
            std::set<Value*>& get_live_in(){return live_in;}
            // 获取该BasicBlock出口处的活跃变量集合
            std::set<Value*>& get_live_out(){return live_out;}
            */
            std::map<BasicBlock*, std::set<Value*>> FuncBBdef;
            std::map<BasicBlock*, std::set<Value*>> FuncBBuse;
            std::map<BasicBlock*, std::set<PhiInst*>> FuncBBphi;
            for (auto bb : func_->get_basic_blocks()) { // 对每一个基本块
                std::set<Value*> use_B;
                std::set<Value*> def_B;
                std::set<PhiInst*> PIN;
                auto globals = this->module->get_global_variable();
                for (auto inst : bb->get_instructions()) {
                    auto instType = inst->get_instr_type();
                    if(instType == Instruction::OpID::alloca){
                        def_B.insert(static_cast<Value*>(inst));
                    }
                    else if(instType == Instruction::OpID::load){
                        // <result> = load <type>, <type>* <pointer>
                        if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end())  //  && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end()
                            use_B.insert(inst->get_operand(0));
                        def_B.insert(static_cast<Value*>(inst));
                    }
                    else if(instType == Instruction::OpID::store){
                        // store <type> <value>, <type>* <pointer>
                        if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end())
                            use_B.insert(inst->get_operand(0));
                        if (def_B.find(inst->get_operand(1)) == def_B.end() && inst->get_operand(1)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(1)) == globals.end())
                            use_B.insert(inst->get_operand(1));
                    }
                    else if (instType == Instruction::OpID::add || instType == Instruction::OpID::fadd || instType == Instruction::OpID::sub || instType == Instruction::OpID::fsub || instType == Instruction::OpID::mul || instType == Instruction::OpID::fmul || instType == Instruction::OpID::sdiv || instType == Instruction::OpID::fdiv || instType == Instruction::OpID::srem) {
                        // 算数
                        // static BinaryInst *create_add(Value *v1, Value *v2, BasicBlock *bb, Module *m);
                        // 若在定义前使用，则加入 use_B
                        // auto dest = inst->get_operand(1)->get_name();   // 获得编号为 1 的操作数（类型为 Value * 指向某个数）
                        if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end())                
                            use_B.insert(inst->get_operand(0));
                        if (def_B.find(inst->get_operand(1)) == def_B.end() && inst->get_operand(1)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(1)) == globals.end())
                            use_B.insert(inst->get_operand(1));
                        // Value *tmp_val = nullptr;
                        // tmp_val = builder->create_iadd(l_val, r_val);
                        def_B.insert(static_cast<Value*>(inst));
                    }
                    else if (instType == Instruction::OpID::cmp || instType == Instruction::OpID::fcmp) {
                        // 比较
                        if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end())
                            use_B.insert(inst->get_operand(0));
                        if (def_B.find(inst->get_operand(1)) == def_B.end() && inst->get_operand(1)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(1)) == globals.end())
                            use_B.insert(inst->get_operand(1));
                        def_B.insert(static_cast<Value*>(inst));
                    }
                    else if (instType == Instruction::OpID::ret) {
                        // ret <type> <value>
                        if (inst->get_num_operand() > 0) {
                            if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end()) {
                                use_B.insert(inst->get_operand(0));
                            }
                        }
                    }
                    else if(instType == Instruction::OpID::br){
                        // br i1 <cond>, label <iftrue>, label <iffalse>
                        if(inst->get_num_operand() > 1){
                            if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end()) {
                                use_B.insert(inst->get_operand(0));
                            }
                        }
                    }
                    else if(instType == Instruction::OpID::phi){
                        def_B.insert(static_cast<Value*>(inst));
                        PIN.insert(static_cast<PhiInst*>(inst));
                    }
                    else if(instType == Instruction::OpID::call){
                        // <result> = call <return ty> <func name>(<function args>) 
                        // for (int i = 1; i < num_ops; i++) {
                        //   set_operand(i, args[i-1]);
                        // }
                        for(int i = 1; i < inst->get_num_operand(); i++){
                            if (def_B.find(inst->get_operand(i)) == def_B.end() && inst->get_operand(i)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(i)) == globals.end()) {
                                use_B.insert(inst->get_operand(i));
                            }
                        }
                        if(!inst->get_type()->is_void_type()){
                            def_B.insert(static_cast<Value*>(inst));
                        }
                    }
                    else if(instType == Instruction::OpID::getelementptr){
                        // <result> = getelementptr <type>, <type>* <ptrval> [, <type> <idx>]
                        for (int i = 0; i < inst->get_num_operand(); i++) {
                            if (def_B.find(inst->get_operand(i)) == def_B.end() && inst->get_operand(i)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(i)) == globals.end())
                                use_B.insert(inst->get_operand(i));
                        }
                        def_B.insert(static_cast<Value*>(inst));
                    }
                    else if(instType == Instruction::OpID::zext){
                        // <result> = zext <type> <value> to <type2>
                        if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end())
                            use_B.insert(inst->get_operand(0));
                        def_B.insert(static_cast<Value*>(inst));
                    }
                    else if(instType == Instruction::OpID::fptosi){
                        // <result> = zext <type> <value> to <type2>
                        if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end())
                            use_B.insert(inst->get_operand(0));
                        def_B.insert(static_cast<Value*>(inst));
                    }
                    else if(instType == Instruction::OpID::sitofp){
                        // <result> = zext <type> <value> to <type2>
                        if (def_B.find(inst->get_operand(0)) == def_B.end() && inst->get_operand(0)->get_name()!="" && std::find(globals.begin(),globals.end(),inst->get_operand(0)) == globals.end())
                            use_B.insert(inst->get_operand(0));
                        def_B.insert(static_cast<Value*>(inst));
                    }
                }
                bb->set_live_in(use_B);
                bb->set_live_out(std::set<Value*>());
                FuncBBdef.insert(make_pair(bb, def_B));
                FuncBBuse.insert(make_pair(bb, use_B));
                FuncBBphi.insert(make_pair(bb, PIN));
            }

            // 根据数据流方程迭代，直到不发生改变
            bool ChangeFlag = true;
            while(ChangeFlag){
                ChangeFlag = false;
                for (auto bb : func_->get_basic_blocks()) { // 对每一个基本块
                    std::set<Value*> OUT;
                    // OUT[B] = ∪S是B的后继 { IN[S] ∪ S的PIN中来源于B的活跃}
                    for(auto succ_bb:bb->get_succ_basic_blocks()){
                        std::set_union(OUT.begin(), OUT.end(), succ_bb->get_live_in().begin(), succ_bb->get_live_in().end(), inserter(OUT, OUT.begin())); // 并
                        std::set<PhiInst*> succ_phi = FuncBBphi[succ_bb];
                        for(auto phi:succ_phi){
                            for (int i = 0; i < phi->get_num_operand(); i += 2) {
                                // %0 = phi [%op1, %bb1], [%op2, %bb2]
                                if (phi->get_operand(i+1) == bb) {
                                    // 当前后继块的该phi指令用到的活跃变量来自于当前块 
                                    OUT.insert(phi->get_operand(i));
                                }
                            }
                        }
                    }
                    if (bb->get_live_out() != OUT) {
                        ChangeFlag = true;
                        bb->set_live_out(OUT);
                    }
                    std::set<Value*> IN;
                    // IN[B] = use_B ∪ (OUT[B]-def_B)
                    std::set_difference(OUT.begin(), OUT.end(), FuncBBdef[bb].begin(), FuncBBdef[bb].end(), inserter(IN, IN.begin()));
                    std::set_union(IN.begin(), IN.end(), FuncBBuse[bb].begin(), FuncBBuse[bb].end(), inserter(IN, IN.begin()));
 
                    if (bb->get_live_in() != IN) {
                        ChangeFlag = true;
                        bb->set_live_in(IN);
                    }  
                }
            }

            for (auto bb : func_->get_basic_blocks()){
                auto IN = bb->get_live_in();
                for (auto phi : FuncBBphi[bb]) {
                    for (int i = 0; i < phi->get_num_operand(); i += 2) {
                        IN.insert(phi->get_operand(i));
                    }
                }
                bb->set_live_in(IN);
            }

            
            for(auto bb : func_->get_basic_blocks()){
                auto IN = bb->get_live_in();
                std::vector<Value *> deleteV;
                for (auto inst : IN)
                {
                    if(inst->get_name()==""){
                        deleteV.push_back(inst);
                    }
                }
                for (auto &&inst : deleteV)
                {
                    IN.erase(inst);
                }
                bb->set_live_in(IN);
                auto OUT = bb->get_live_out();
                deleteV.clear();
                for (auto inst : OUT)
                {
                    if(inst->get_name()==""){
                        deleteV.push_back(inst);
                    }
                }
                for (auto &&inst : deleteV)
                {
                    OUT.erase(inst);
                }
                bb->set_live_out(OUT);
            }
            
        }
    }

    /*
    for (auto &func: module->get_functions()) {
        auto globals = this->module->get_global_variable();
        for (auto &bb: func->get_basic_blocks()) {
            std::cout << bb->get_name() << std::endl;
            auto &in = bb->get_live_in();
            auto &out = bb->get_live_out();
            auto sorted_in = sort_by_name(in);
            auto sorted_out = sort_by_name(out);
            std::cout << "in:\n";
            for (auto in_v: sorted_in) {
                if(in_v->get_name()=="") continue;
                if(std::find(globals.begin(), globals.end(), in_v) != globals.end()) continue;
                std::cout << in_v->get_name() << " ";
            }
            std::cout << "\n";
            std::cout << "out:\n";
            for (auto out_v: sorted_out) {
                if(out_v->get_name()=="") continue;
                if(std::find(globals.begin(), globals.end(), out_v) != globals.end()) continue;
                std::cout << out_v->get_name() << " ";
            }
            std::cout << "\n";
        }
    }
    */

    //  请不要修改该代码，在被评测时不要删除该代码
    dump();
    //
    return ;
}

void ActiveVar::dump() {
    std::fstream f;
    f.open(avdump, std::ios::out);
    for (auto &func: module->get_functions()) {
        for (auto &bb: func->get_basic_blocks()) {
            f << bb->get_name() << std::endl;
            auto &in = bb->get_live_in();
            auto &out = bb->get_live_out();
            auto sorted_in = sort_by_name(in);
            auto sorted_out = sort_by_name(out);
            f << "in:\n";
            for (auto in_v: sorted_in) {
                if(in_v->get_name()=="") continue;
                f << in_v->get_name() << " ";
            }
            f << "\n";
            f << "out:\n";
            for (auto out_v: sorted_out) {
                if(out_v->get_name()=="") continue;
                f << out_v->get_name() << " ";
            }
            f << "\n";
        }
    }
    f.close();
}

bool ValueCmp(Value* a, Value* b) {
    return a->get_name() < b->get_name();
}

std::vector<Value*> sort_by_name(std::set<Value*> &val_set) {
    std::vector<Value*> result;
    result.assign(val_set.begin(), val_set.end());
    std::sort(result.begin(), result.end(), ValueCmp);
    return result;
}