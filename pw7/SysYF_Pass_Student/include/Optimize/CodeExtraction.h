#ifndef SYSYF_CODEEXTRA_H
#define SYSYF_CODEEXTRA_H

#include "Pass.h"

class CodeExtraction : public Pass {
public:
    CodeExtraction(Module *module): Pass(module){}
    void execute() final;
    void cope_loop(std::map<BasicBlock*, std::set<BasicBlock*> > &loop);    // 处理循环
    void find_invar(std::set<BasicBlock*> &loop_bb, std::set<Instruction*> &invar); // 寻找循环不变量
    BinaryInst* copy(Instruction* inst);    // 拷贝二元指令
    const std::string get_name() const { return name; }

private:
    const std::string name = "CodeExtraction";

};

#endif