#ifndef SYSYF_CSEGLO_H
#define SYSYF_CSEGLO_H

#include "Pass.h"
#include "Module.h"
#include <list>

typedef struct Expression {
    Instruction::OpID opcode;
    std::vector<Value *> operands;
    std::list<Instruction *> Finsts;
    Expression(Instruction *inst){
        Finsts.push_back(inst);
        opcode = inst->get_instr_type();
        operands = inst->get_operands();
    }
    bool operator==(const Expression &expr2) const{
        if(opcode != expr2.opcode) // opcode
            return false;
        auto operands1 = operands;
        auto operands2 = expr2.operands;
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
}Expression;

class CSEglo : public Pass
{
public:
    CSEglo(Module *module) : Pass(module) {}
    bool ExprEqual(Expression expr1, Expression expr2);
    void AvalExpr(Function *func);
    void AddFirstCal(Function *func);
    void DelCSE(Function *func);
    void execute() final;
    const std::string get_name() const override {return name;}
private:
    std::list<Expression> AllExprInsts; // 所有是表达式
    // 下面，分别是每个基本块产生、注销、入口、出口的可用表达式集合
    std::map<BasicBlock*, std::set<Instruction *>> EGen; 
    std::map<BasicBlock*, std::set<Instruction *>> EKill;
    std::map<BasicBlock*, std::set<Instruction *>> EIN;
    std::map<BasicBlock*, std::set<Instruction *>> EOUT;
    const std::string name = "CSEglo";
};

#endif  // SYSYF_CSEGLO_H