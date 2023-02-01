#ifndef SYSYF_STRENTHRED_H
#define SYSYF_STRENTHRED_H

#include "Pass.h"
//#include "Module.h"

class Var_Tri {
public:
    Value* val_;
    int  mul, add;
    Var_Tri(Value* val_, int  mul, int  add): val_(val_), mul(mul), add(add) {}
    Var_Tri(){val_=NULL; mul=0; add=0;}
};  // 三元组，name 是变量名，mul 和 add 分别是加法和乘法因子

class StrengthReduction : public Pass{
public:
    StrengthReduction(Module* module): Pass(module){}
    void execute() final;
    void easy_sub(Function *f);    // 用于将乘/除2的次幂换成移位
    void find_loops(Function *f, std::map<BasicBlock*, std::set<BasicBlock*> > &loop);   // 寻找循环
    void reverse_dfs(BasicBlock* bb, std::set<BasicBlock*>& visited);   // 寻找归纳变量
    bool is_dom(BasicBlock* bb1, BasicBlock* bb2);  // 判断 bb2 是不是 bb1 的支配节点  
    void cal_dom_set(Function *f);  // 计算每个节点的支配节点点集 
    void find_inductive_var(std::map<Value*, Var_Tri> &var, BasicBlock* entry, std::map<BasicBlock*, std::set<BasicBlock*> > &loop);  // 用于寻找归纳变量
    void cope_loop(std::map<BasicBlock*, std::set<BasicBlock*> > &loop);   // 处理循环以对归纳变量进行强度削弱
    void constant_sub(Function *f);    // 处理 i-i、i*1、i*0、i+0 等的情况
    const std::string get_name() const { return name; }

private:
    const std::string name = "StrengthReduction";
    std::map<BasicBlock*, std::set<BasicBlock*> > dom;  // 从 bb 到其支配节点的映射
       
    
};


#endif