#pragma once

#include "Pass.h"
#include "Module.h"

#include <vector>
#include <map>
#include <set>
#include <tuple>

enum LatticeType
{
    Unknown,
    Constant,
    NotConstant
};

class Lattice
{
public:
    Lattice() = default; // 基础构造用于STL，平时不应使用
    explicit Lattice(LatticeType t) : type(t) {}
    explicit Lattice(int i_) : type(LatticeType::Constant), isInt(true), i(i_) {}
    explicit Lattice(float f_) : type(LatticeType::Constant), isInt(false), f(f_) {}
    // 在 phi 指令中对 Lattice 之间进行 meet
    Lattice operator+(Lattice const &other) const;
    // 两个操作数的 Lattice 判断操作后的值其 Lattice 类型是什么
    LatticeType operator*(Lattice const &other) const;
    bool operator==(Lattice const &other) const;
    bool isZero() const { return (isInt && i == 0) || (!isInt && f == 0); }
    LatticeType getType() const { return type; }
    std::tuple<bool, int, float> getConstant() const { return {isInt, i, f}; }
    int getInt() const { return i; }
    float getFloat() const { return f; }

private:
    LatticeType type;
    bool isInt = false;
    int i = 0;
    float f = 0;
};

class constPro : public Pass
{
public:
    using Pass::Pass;
    void execute() final;
    const std::string get_name() const override { return name; }

private:
    const std::string name = "constPro";
    Function *fun;
    size_t flowIter = 0;
    size_t SSAIter = 0;
    // Value * 到 Lattice 的映射，存储每个Value对应的Lattice
    std::map<Value *, Lattice> latCell;
    // SSA 的 worklist
    std::vector<Instruction *> SSAWL;
    // CFG edge 的 worklist
    std::vector<std::pair<BasicBlock *, BasicBlock *>> flowWL;
    // 存储已经处理过的 CFG edge ，排除重复处理的情况
    std::set<std::pair<BasicBlock *, BasicBlock *>> execFlags;

    void initialize();
    bool hasRet(Instruction *op);
    void removePhi(BasicBlock *bb, BasicBlock *suc);
    void visitPhi(Instruction *phi);
    void visitInst(Instruction *ins);
    void judgeConst(Instruction *ins, size_t i);
    void removeDeadBB(BasicBlock *bb);
};