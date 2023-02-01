#include "CSEglo.h"
#include <algorithm>
#include <map>

void CSEglo::execute() {
    module->set_print_name();
    // 全局的公共子表达式删除，考虑不同Block之间
    // 首先要做可用表达式分析

    for (auto& func : this->module->get_functions()) {
        if (func->get_basic_blocks().empty()) continue;
        // 每个func做可用表达式分析
        AllExprInsts.clear();
        EGen.clear(); EKill.clear(); EIN.clear(); EOUT.clear();
        AvalExpr(func);
        AddFirstCal(func);
        DelCSE(func);
    }
}

void CSEglo::AvalExpr(Function* func) {
    bool FindFlag;
    for (auto bb : func->get_basic_blocks()) {
        for (auto inst : bb->get_instructions()) {
            if (inst->is_binary() || inst->is_float_binary()) {
                FindFlag = false;
                auto expr = Expression(inst);
                for (auto expr0 : AllExprInsts) {
                    if (ExprEqual(expr, expr0)) {
                        FindFlag = true; break;
                    }
                }
                if (!FindFlag) AllExprInsts.push_back(expr);
            }
        }
    }

    // 计算每个基本快的 e_gen 和 e_kill
    for (auto bb : func->get_basic_blocks()) {
        std::set<Instruction*> e_gen;
        std::set<Instruction*> e_kill;
        std::vector<Instruction*> delete_E;
        for (auto inst : bb->get_instructions()) {
            if (inst->is_binary() || inst->is_float_binary()) {
                // z = x op y
                auto expr = Expression(inst);
                // 处理 e_gen
                for (auto expr0 : AllExprInsts) {
                    if (ExprEqual(expr, expr0)) {
                        e_gen.insert(expr0.Finsts.front());
                        break;
                    }
                }
                delete_E.clear();
                for (auto inst1 : e_gen) {
                    for (auto operand1 : inst1->get_operands()) {
                        if (operand1 == static_cast<Value*>(inst))
                            delete_E.push_back(inst1);
                        //e_gen.erase(expr1);
                    }
                }
                for (auto&& inst1 : delete_E)
                {
                    e_gen.erase(inst1);
                }
                // 处理 e_kill
                delete_E.clear();
                for (auto inst0 : e_kill) {
                    auto expr0 = Expression(inst0);
                    if (ExprEqual(expr, expr0)) {
                        delete_E.push_back(inst0);
                        //e_kill.erase(expr0);
                    }
                }
                for (auto&& inst0 : delete_E)
                {
                    e_kill.erase(inst0);
                }
                for (auto expr0 : AllExprInsts) {
                    for (auto operand0 : expr0.operands) {
                        if (operand0 == static_cast<Value*>(inst)) e_kill.insert(expr0.Finsts.front());
                    }
                }
            }
        }
        EGen.insert({ bb, e_gen });
        EKill.insert({ bb, e_kill });
    }

    // 计算每个基本块的IN和OUT
    for (auto bb : func->get_basic_blocks()) {
        std::set<Instruction*> IN;
        std::set<Instruction*> OUT;
        for (auto expr : AllExprInsts) {
            OUT.insert(expr.Finsts.front());
            IN.insert(expr.Finsts.front());
        }
        EIN.insert({ bb, IN });
        EOUT.insert({ bb, OUT });
    }
    // EOUT[func->get_entry_block()].clear();
    bool ChangeFlag = true;
    while (ChangeFlag) {
        ChangeFlag = false;
        for (auto bb : func->get_basic_blocks()) {
            // if (bb == func->get_entry_block()) continue;
            std::set<Instruction*> IN;
            std::set<Instruction*> OUT;
            for (auto expr : AllExprInsts) {
                IN.insert(expr.Finsts.front());
            }
            // IN[B] = ∩ OUT[P]，其中P是B的前驱
            for (auto pre_bb : bb->get_pre_basic_blocks()) {
                std::set_intersection(IN.begin(), IN.end(), EOUT[pre_bb].begin(), EOUT[pre_bb].end(), inserter(IN, IN.begin()));
            }
            EIN[bb] = IN;
            std::set_difference(IN.begin(), IN.end(), EKill[bb].begin(), EKill[bb].end(), inserter(OUT, OUT.begin()));
            std::set_union(EGen[bb].begin(), EGen[bb].end(), OUT.begin(), OUT.end(), inserter(OUT, OUT.begin()));
            if (OUT != EOUT[bb]) {
                ChangeFlag = true;
                EOUT[bb] = OUT;
            }
        }
    }

}

void CSEglo::AddFirstCal(Function* func) {
    bool FindFlag;
    std::set<Instruction*> e_gen;
    std::set<Instruction*> e_kill;
    std::set<Instruction*> OUT;
    std::vector<Instruction*> delete_E;
    for (auto bb : func->get_basic_blocks()) {
        e_gen.clear(); e_kill.clear(); OUT.clear(); delete_E.clear();
        for (auto inst : bb->get_instructions()) {
            if (inst->is_binary() || inst->is_float_binary()) {
                auto expr = Expression(inst);
                for (auto expr0 : AllExprInsts) {
                    if (ExprEqual(expr, expr0) && std::find(expr0.Finsts.begin(), expr0.Finsts.end(), inst) == expr0.Finsts.end()) {
                        // 该指令不在该表达式的首次计算集合
                        // 并且到当前指令的可用表达式中，没有包含当前指令所计算的表达式
                        FindFlag = false;
                        for (auto inst1 : EIN[bb]) {
                            auto expr1 = Expression(inst1);
                            if (ExprEqual(expr, expr1)) {
                                FindFlag = true; break;
                            }
                        }
                        if (FindFlag == true) continue;
                        for (auto inst2 : OUT) {
                            auto expr2 = Expression(inst2);
                            if (ExprEqual(expr, expr2)) {
                                FindFlag = true; break;
                            }
                        }
                        if (FindFlag == false) {
                            auto iter = std::find(AllExprInsts.begin(), AllExprInsts.end(), expr0);
                            iter->Finsts.push_back(inst);
                        }
                    }
                }
                // 处理 e_gen
                for (auto expr0 : AllExprInsts) {
                    if (ExprEqual(expr, expr0)) {
                        e_gen.insert(expr0.Finsts.front());
                        break;
                    }
                }
                delete_E.clear();
                for (auto inst1 : e_gen) {
                    for (auto operand1 : inst1->get_operands()) {
                        if (operand1 == static_cast<Value*>(inst))
                            delete_E.push_back(inst1);
                        //e_gen.erase(expr1);
                    }
                }
                for (auto&& inst1 : delete_E)
                {
                    e_gen.erase(inst1);
                }
                // 处理 e_kill
                delete_E.clear();
                for (auto inst0 : e_kill) {
                    auto expr0 = Expression(inst0);
                    if (ExprEqual(expr, expr0)) {
                        delete_E.push_back(inst0);
                        //e_kill.erase(expr0);
                    }
                }
                for (auto&& inst0 : delete_E)
                {
                    e_kill.erase(inst0);
                }
                for (auto expr0 : AllExprInsts) {
                    for (auto operand0 : expr0.operands) {
                        if (operand0 == static_cast<Value*>(inst)) e_kill.insert(expr0.Finsts.front());
                    }
                }
                // OUT
                std::set_difference(EIN[bb].begin(), EIN[bb].end(), e_kill.begin(), e_kill.end(), inserter(OUT, OUT.begin()));
                std::set_union(e_gen.begin(), e_gen.end(), OUT.begin(), OUT.end(), inserter(OUT, OUT.begin()));
            }
        }
    }
}

void CSEglo::DelCSE(Function* func) {
    std::vector<Instruction*> delete_I;
    for (auto bb : func->get_basic_blocks()) {
        delete_I.clear();
        for (auto inst : bb->get_instructions()) {
            if (inst->is_binary() || inst->is_float_binary()) {
                auto expr = Expression(inst);
                // 如果当前指令含有公共表达式
                // 并且不是第一次被定值
                for (auto expr0 : AllExprInsts) {
                    if (ExprEqual(expr, expr0) && std::find(expr0.Finsts.begin(), expr0.Finsts.end(), inst) == expr0.Finsts.end()) {
                        if (expr0.Finsts.size() == 1) {
                            inst->replace_all_use_with(expr0.Finsts.front());
                        }
                        else {
                            auto phi0 = PhiInst::create_phi(inst->get_type(), bb);
                            phi0->set_lval(inst);
                            bb->add_instr_begin(phi0);
                            for (auto inst1 : expr0.Finsts) {
                                phi0->add_phi_pair_operand(inst1, inst1->get_parent());
                            }
                            inst->replace_all_use_with(phi0);
                        }
                        // bb->delete_instr(inst);
                        delete_I.push_back(inst);
                    }
                }
            }
        }
        for (auto&& inst : delete_I) {
            bb->delete_instr(inst);
        }
    }
}

bool CSEglo::ExprEqual(Expression expr1, Expression expr2) {
    if (expr1.opcode != expr2.opcode) // opcode
        return false;
    auto operands1 = expr1.operands;
    auto operands2 = expr2.operands;
    if (operands1 == operands2)
        return true;
    else {
        if (operands1.size() != operands2.size()) return false;
        for (int i = 0; i < operands1.size(); i++) {
            auto constint1 = dynamic_cast<ConstantInt*>(operands1[i]);
            auto constint2 = dynamic_cast<ConstantInt*>(operands2[i]);
            auto constfloat1 = dynamic_cast<ConstantFloat*>(operands1[i]);
            auto constfloat2 = dynamic_cast<ConstantFloat*>(operands1[i]);
            if (constint1 && constint2 && constint1->get_value() != constint2->get_value()) return false;
            else if (constfloat1 && constfloat2 && constfloat1->get_value() != constfloat2->get_value()) return false;
            else if (operands1[i] != operands2[i]) return false;
        }
        return true;
    }
}
