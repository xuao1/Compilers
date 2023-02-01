#include "constPro.h"

#include <list>
#include <algorithm>

using namespace std;

Lattice Lattice::operator+(Lattice const &other) const
{
    switch (type)
    {
    case Unknown:
        return other;
    case NotConstant:
        return Lattice(NotConstant);
    default:
        switch (other.type)
        {
        case Unknown:
            return *this;
        case NotConstant:
            return Lattice(NotConstant);
        default:
            if (*this == other)
            {
                return *this;
            }
            return Lattice(NotConstant);
        }
    }
}

LatticeType Lattice::operator*(Lattice const &other) const
{
    if (type == NotConstant || other.type == NotConstant)
        return NotConstant;
    if (type == Unknown || other.type == Unknown)
        return Unknown;
    return Constant;
}

bool Lattice::operator==(Lattice const &other) const
{
    if (type == Constant && other.type == Constant)
    {
        if (isInt == other.isInt)
        {
            if ((isInt == true && i == other.i) ||
                (isInt == false && f == other.f))
            {
                return true;
            }
        }
        else
        { // 两者均为零的情况
            if (isZero() && other.isZero())
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        return type == other.type;
    }
}

bool constPro::hasRet(Instruction *op)
{
    if (auto type = op->get_instr_type();
        type == Instruction::OpID::ret ||
        type == Instruction::OpID::br ||
        type == Instruction::OpID::alloca ||
        type == Instruction::OpID::store)
        return false;
    return true;
}

void constPro::execute()
{
    // 该模块实现 稀疏条件传播
    // 即 sparse conditional constant propagation
    for (auto &&workFun : module->get_functions())
    {
        if (workFun->is_declaration())
            continue;
        fun = workFun;
        initialize();
        set<BasicBlock *> firstVisitBB;

        flowIter = 0;
        SSAIter = 0;
        while (flowIter < flowWL.size())
        {
            for (; flowIter < flowWL.size(); flowIter++)
            {
                if (auto bbEdge = flowWL[flowIter];
                    execFlags.insert(bbEdge).second)
                {
                    auto bb = bbEdge.second;
                    // 处理 phi 指令
                    for (auto inst : bb->get_instructions())
                    {
                        if (inst->get_instr_type() != Instruction::OpID::phi)
                            break;
                        visitPhi(inst);
                    }
                    // 第一次 visit BB 时，处理每一条非 phi 指令
                    if (firstVisitBB.insert(bb).second)
                    {
                        for (auto inst : bb->get_instructions())
                        {
                            if (inst->get_instr_type() == Instruction::OpID::phi)
                                continue;
                            visitInst(inst);
                        }
                    }
                    // 如果存在单一的后继，加入处理
                    if (auto succs = bb->get_succ_basic_blocks();
                        succs.size() == 1)
                    {
                        flowWL.push_back({bb, *succs.begin()});
                    }
                }
            }
            for (; SSAIter < SSAWL.size(); SSAIter++)
            {
                auto inst = SSAWL[SSAIter];
                if (inst->get_instr_type() == Instruction::OpID::phi)
                    visitPhi(inst);
                else
                    visitInst(inst);
            }
        }
        // 完成最后的常量替换与 branch 死代码删除
        list<BasicBlock *> deleteBBList;
        for (auto &&bb : fun->get_basic_blocks())
        {
            list<Instruction *> deleteList;
            for (auto &&inst : bb->get_instructions())
            {
                if (hasRet(inst))
                {
                    auto lat = latCell[inst];
                    if (lat.getType() == Constant)
                    {
                        Value *new_value;
                        auto [isInt, i, f] = lat.getConstant();
                        if (isInt)
                        {
                            if (i == 0)
                            {
                                new_value = ConstantZero::get(Type::get_int32_type(module), module);
                            }
                            else
                            {
                                new_value = ConstantInt::get(i, module);
                            }
                            latCell.insert({new_value, Lattice(i)});
                        }
                        else
                        {
                            if (f == .0f)
                            {
                                new_value = ConstantZero::get(Type::get_float_type(module), module);
                            }
                            else
                            {
                                new_value = ConstantFloat::get(f, module);
                            }
                            latCell.insert({new_value, Lattice(f)});
                        }
                        inst->replace_all_use_with(new_value);
                        deleteList.push_back(inst);
                    }
                }
                else
                {
                    // 删除死代码分支
                    if (inst->get_instr_type() == Instruction::OpID::br)
                    {
                        auto branch = dynamic_cast<BranchInst *>(inst);
                        if (branch->is_cond_br())
                        {
                            auto condLat = latCell[branch->get_operand(0)];
                            if (condLat.getType() == Constant)
                            {
                                auto bb = branch->get_parent();
                                auto if_true = dynamic_cast<BasicBlock *>(branch->get_operand(1));
                                auto if_false = dynamic_cast<BasicBlock *>(branch->get_operand(2));
                                BasicBlock *trueBB;
                                BasicBlock *falseBB;
                                if (condLat.isZero())
                                {
                                    trueBB = if_false;
                                    falseBB = if_true;
                                }
                                else
                                {
                                    trueBB = if_true;
                                    falseBB = if_false;
                                }

                                // 处理前驱后继关系
                                bb->remove_succ_basic_block(trueBB);
                                bb->remove_succ_basic_block(falseBB);
                                falseBB->remove_pre_basic_block(bb);
                                trueBB->remove_pre_basic_block(bb);

                                removePhi(bb, falseBB);

                                deleteList.push_back(branch);
                                BranchInst::create_br(trueBB, bb);
                                deleteBBList.push_back(falseBB);
                            }
                        }
                    }
                }
            }
            for (auto inst : deleteList)
            {
                bb->delete_instr(inst);
            }
        }
        for (auto &&deleteBB : deleteBBList)
        {
            removeDeadBB(deleteBB);
        }
    }
}

void constPro::initialize()
{
    SSAWL.clear();
    flowWL.clear();
    latCell.clear();
    execFlags.clear();

    for (auto &&arg : fun->get_args())
    {
        latCell.insert({arg, Lattice(NotConstant)});
    }
    for (auto &&bb : fun->get_basic_blocks())
    {
        for (auto &&inst : bb->get_instructions())
        {
            if (hasRet(inst))
                latCell.insert({inst, Lattice(Unknown)});
        }
    }
    // 除了 constant 和 全局变量 以外的所有引用均已建立 Lattice 条目

    flowWL.push_back({nullptr, fun->get_entry_block()});
}

void constPro::visitPhi(Instruction *phi)
{
    auto inst = dynamic_cast<PhiInst *>(phi);
    auto operands = inst->get_operands();
    auto before = latCell[inst];
    auto &lat = latCell[inst];
    for (int i = 0, end = operands.size(); i < end; i += 2)
    {
        auto val = operands[i];
        auto inBB = dynamic_cast<BasicBlock *>(operands[i + 1]);
        if (execFlags.find({inBB, inst->get_parent()}) != execFlags.end())
        {
            judgeConst(inst, i);
            if (auto latOther = latCell.find(val); latOther != latCell.end())
            {
                lat = lat + latOther->second;
            }
        }
    }
    if (!(before == lat))
    {
        for (auto use : inst->get_use_list())
        {
            Instruction *use_inst = dynamic_cast<Instruction *>(use.val_);
            if (find(SSAWL.begin() + SSAIter, SSAWL.end(), use_inst) == SSAWL.end())
                SSAWL.push_back(use_inst);
        }
    }
}

void constPro::visitInst(Instruction *inst)
{
    auto insType = inst->get_instr_type();
    if (!hasRet(inst))
    {
        // 判断 BR 指令
        if (insType == Instruction::OpID::br)
        {
            // 仅处理含有分支的 br 指令
            auto branch = dynamic_cast<BranchInst *>(inst);
            if (branch->is_cond_br())
            {
                auto cond = branch->get_operand(0);
                auto bb = branch->get_parent();
                auto if_true = dynamic_cast<BasicBlock *>(branch->get_operand(1));
                auto if_false = dynamic_cast<BasicBlock *>(branch->get_operand(2));
                auto condLat = latCell[cond];
                switch (condLat.getType())
                {
                case Constant:
                    if (condLat.isZero())
                    {
                        if (find(flowWL.begin() + flowIter, flowWL.end(), make_pair(bb, if_false)) == flowWL.end())
                            flowWL.push_back({bb, if_false});
                    }
                    else
                    {
                        if (find(flowWL.begin() + flowIter, flowWL.end(), make_pair(bb, if_true)) == flowWL.end())
                            flowWL.push_back({bb, if_true});
                    }
                    break;
                case NotConstant:
                    if (find(flowWL.begin() + flowIter, flowWL.end(), make_pair(bb, if_false)) == flowWL.end())
                        flowWL.push_back({bb, if_false});
                    if (find(flowWL.begin() + flowIter, flowWL.end(), make_pair(bb, if_true)) == flowWL.end())
                        flowWL.push_back({bb, if_true});
                    break;
                default:
#ifdef DEBUG
                    // 这种情况不应该出现
                    cout << "An unforeseen situation\n";
#endif
                    break;
                }
            }
        }
        return;
    }

    auto before = latCell[inst];
    auto &lat = latCell[inst];

    // 完成每一条指令的处理
    switch (insType)
    {
    case Instruction::OpID::add:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getInt() + rhs.getInt());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::sub:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getInt() - rhs.getInt());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::mul:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getInt() * rhs.getInt());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::sdiv:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getInt() / rhs.getInt());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::srem:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getInt() % rhs.getInt());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;

    case Instruction::OpID::fadd:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getFloat() + rhs.getFloat());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::fsub:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getFloat() - rhs.getFloat());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::fmul:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getFloat() * rhs.getFloat());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::fdiv:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getFloat() / rhs.getFloat());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;

    case Instruction::OpID::load:
    {
        // 暂时不考虑局部数组和全局变量的优化
        lat = Lattice(NotConstant);
    }
    break;

    case Instruction::OpID::cmp:
    {
        auto dynIns = dynamic_cast<CmpInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            switch (dynIns->get_cmp_op())
            {
            case CmpInst::CmpOp::EQ:
                lat = Lattice(lhs.getInt() == rhs.getInt());
                break;
            case CmpInst::CmpOp::NE:
                lat = Lattice(lhs.getInt() != rhs.getInt());
                break;
            case CmpInst::CmpOp::GT:
                lat = Lattice(lhs.getInt() > rhs.getInt());
                break;
            case CmpInst::CmpOp::GE:
                lat = Lattice(lhs.getInt() >= rhs.getInt());
                break;
            case CmpInst::CmpOp::LT:
                lat = Lattice(lhs.getInt() < rhs.getInt());
                break;
            case CmpInst::CmpOp::LE:
                lat = Lattice(lhs.getInt() <= rhs.getInt());
                break;
            default:
                break;
            }
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::fcmp:
    {
        auto dynIns = dynamic_cast<FCmpInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            switch (dynIns->get_cmp_op())
            {
            case FCmpInst::CmpOp::EQ:
                lat = Lattice(lhs.getFloat() == rhs.getFloat());
                break;
            case FCmpInst::CmpOp::NE:
                lat = Lattice(lhs.getFloat() != rhs.getFloat());
                break;
            case FCmpInst::CmpOp::GT:
                lat = Lattice(lhs.getFloat() > rhs.getFloat());
                break;
            case FCmpInst::CmpOp::GE:
                lat = Lattice(lhs.getFloat() >= rhs.getFloat());
                break;
            case FCmpInst::CmpOp::LT:
                lat = Lattice(lhs.getFloat() < rhs.getFloat());
                break;
            case FCmpInst::CmpOp::LE:
                lat = Lattice(lhs.getFloat() <= rhs.getFloat());
                break;
            default:
                break;
            }
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::call:
    {
        // 暂时不考虑函数之间的联动优化
        lat = Lattice(NotConstant);
    }
    break;
    case Instruction::OpID::getelementptr:
    {
        // 暂时不考虑局部数组的优化
        lat = Lattice(NotConstant);
    }
    break;

    case Instruction::OpID::zext:
    {
        // zext 存在于 icmp 产生的 bool 转 int
        auto dynIns = dynamic_cast<ZextInst *>(inst);
        judgeConst(inst, 0);
        auto val = latCell[dynIns->get_operand(0)];
        lat = val;
    }
    break;
    case Instruction::OpID::fptosi:
    {
        auto dynIns = dynamic_cast<FpToSiInst *>(inst);
        judgeConst(inst, 0);
        auto val = latCell[dynIns->get_operand(0)];
        switch (val.getType())
        {
        case Constant:
            lat = Lattice(static_cast<int>(val.getFloat()));
            break;
        default:
            lat = Lattice(val.getType());
            break;
        }
    }
    break;
    case Instruction::OpID::sitofp:
    {
        auto dynIns = dynamic_cast<SiToFpInst *>(inst);
        judgeConst(inst, 0);
        auto val = latCell[dynIns->get_operand(0)];
        switch (val.getType())
        {
        case Constant:
            lat = Lattice(static_cast<float>(val.getInt()));
            break;
        default:
            lat = Lattice(val.getType());
            break;
        }
    }
    break;
    case Instruction::OpID::shl:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getInt() << rhs.getInt());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    case Instruction::OpID::ashr:
    {
        auto dynIns = dynamic_cast<BinaryInst *>(inst);
        judgeConst(inst, 0);
        judgeConst(inst, 1);
        auto lhs = latCell[dynIns->get_operand(0)];
        auto rhs = latCell[dynIns->get_operand(1)];
        switch (lhs * rhs)
        {
        case Constant:
            lat = Lattice(lhs.getInt() >> rhs.getInt());
            break;
        default:
            lat = Lattice(lhs * rhs);
            break;
        }
    }
    break;
    default:
        break;
    }
    if (!(before == lat))
    {
        for (auto use : inst->get_use_list())
        {
            Instruction *use_inst = dynamic_cast<Instruction *>(use.val_);
            if (find(SSAWL.begin() + SSAIter, SSAWL.end(), use_inst) == SSAWL.end())
                SSAWL.push_back(use_inst);
        }
    }
}

// 加入常数进入 latCell
void constPro::judgeConst(Instruction *ins, size_t n)
{
    // val 为 constant
    auto val = ins->get_operand(n);
    auto intConst = dynamic_cast<ConstantInt *>(val);
    if (intConst)
    {
        latCell.insert({val, Lattice(intConst->get_value())});
        return;
    }
    auto floatConst = dynamic_cast<ConstantFloat *>(val);
    if (floatConst)
    {
        latCell.insert({val, Lattice(floatConst->get_value())});
        return;
    }
    auto zeroConst = dynamic_cast<ConstantZero *>(val);
    if (zeroConst)
    {
        if (zeroConst->get_type()->get_type_id() == Type::TypeID::IntegerTyID)
            latCell.insert({val, Lattice(0)});
        else
            latCell.insert({val, Lattice(.0f)});
        return;
    }
    // 不考虑常数数组
}

void constPro::removeDeadBB(BasicBlock *bb)
{
    if (bb->get_pre_basic_blocks().empty())
    {
        auto succs = list(bb->get_succ_basic_blocks());
        fun->remove(bb);
        for (auto suc : succs)
        {
            // 处理 phi 关系
            removePhi(bb, suc);
            // 对后继块继续判断与处理
            removeDeadBB(suc);
        }
    }
}

// 处理 phi 关系，删除 suc 中的 phi 指令含有 bb 的部分
void constPro::removePhi(BasicBlock *bb, BasicBlock *suc)
{
    vector<Instruction *> deleteInst;
    for (auto &&inst : suc->get_instructions())
    {
        if (inst->get_instr_type() != Instruction::OpID::phi)
            break;
        auto operands = inst->get_operands();
        for (int i = 1, end = operands.size(); i < end; i += 2)
        {
            auto inBB = dynamic_cast<BasicBlock *>(operands[i]);
            if (inBB == bb)
            {
                inst->remove_operands(i - 1, i);
                break;
            }
        }
        if (inst->get_num_operand() == 2)
        {
            inst->replace_all_use_with(inst->get_operand(0));
            deleteInst.push_back(inst);
        }
    }
    for (auto inst : deleteInst)
    {
        suc->delete_instr(inst);
    }
}