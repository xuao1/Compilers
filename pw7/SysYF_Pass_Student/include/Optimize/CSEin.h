#ifndef SYSYF_CSEIN_H
#define SYSYF_CSEIN_H

#include "Pass.h"
#include "Module.h"

class CSEin : public Pass
{
public:
    CSEin(Module *module) : Pass(module) {}
    void execute() final;
    bool ExprEqual(Instruction *inst1, Instruction *inst2);
    const std::string get_name() const override {return name;}
private:
    std::list<Instruction *> ExprInsts; // 是表达式的指令
    const std::string name = "CSEin";
};

#endif  // SYSYF_CSEIN_H