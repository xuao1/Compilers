#include "tailCall.h"
#include <vector>
#include <map>

using namespace std;

void tailCall::execute()
{
    for (auto fun : module->get_functions())
    {
        // 根据函数类型判断是否可能存在尾递归
        if (fun->is_declaration())
            continue;
        auto funcType = fun->get_function_type();
        int paramNum = funcType->param_end() - funcType->param_begin();
        if (paramNum == 0)
            continue;
        if (funcType->get_return_type()->is_void_type())
            continue;
        AllocaInst *retAlloc = nullptr;
        vector<AllocaInst *> paramsAlloc;

        // 在 Mem2Reg 前处理，获取 ret 和所有 arg 的存储地址
        auto entryBB = fun->get_entry_block();
        for (auto inst : entryBB->get_instructions())
        {
            if (inst->get_instr_type() == Instruction::OpID::alloca)
            {
                if (retAlloc == nullptr)
                    retAlloc = static_cast<AllocaInst *>(inst);
                else if (paramsAlloc.size() < paramNum)
                    paramsAlloc.push_back(static_cast<AllocaInst *>(inst));
                else
                    break;
            }
        }

        // 检查尾递归调用
        map<BasicBlock *, StoreInst *> workList;
        for (auto bb : fun->get_basic_blocks())
        {
            // 查找以 ret 结尾的 bb
            if (bb->get_terminator()->get_instr_type() != Instruction::ret)
                continue;
            for (auto preBB : bb->get_pre_basic_blocks())
            {
                for (auto inst : preBB->get_instructions())
                {
                    // 判断尾递归条件
                    if (inst->get_instr_type() == Instruction::OpID::store)
                    {
                        auto storeIns = static_cast<StoreInst *>(inst);
                        auto callIns = dynamic_cast<CallInst *>(storeIns->get_rval());
                        if (callIns)
                        {
                            auto callfunc = static_cast<Function *>(callIns->get_operand(0));
                            if (callfunc == fun)
                            {
                                auto lvalue = dynamic_cast<AllocaInst *>(storeIns->get_lval());
                                if (lvalue == retAlloc)
                                {
                                    workList.insert({preBB, storeIns});
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        // 不存在尾递归，结束该函数的处理
        if (workList.empty())
            continue;

        // 将 entryBB 分裂为两块，后半块为除了 ret和参数的store&alloca，其他局部变量 alloca 以外的指令
        auto entryTail = BasicBlock::create(module, "entryTail", fun);
        auto insIter = entryBB->get_instructions().begin();
        auto insEnd = entryBB->get_instructions().end();
        // 通过位置信息确定位置
        // 1 + 2 * paramNum 分别为 ret 的 alloca 和参数的 alloca & store
        for (int i = 0; i < 1 + 2 * paramNum; i++)
        {
            insIter++;
        }
        auto deleteBegin = insIter;

        // 移动指令
        while (insIter != insEnd)
        {
            auto inst = *(insIter++);
            if (inst->get_instr_type() != Instruction::OpID::alloca)
            {
                entryTail->add_instruction(inst);
                inst->set_parent(entryTail);
            }
        }
        auto modifyedBB = list<BasicBlock *>(entryBB->get_succ_basic_blocks());
        // 处理尾部关系
        for (auto &&succ : modifyedBB)
        {
            entryTail->add_succ_basic_block(succ);
        }

        // 删除被移动的指令
        for (auto &&deleteInst : list<Instruction *>(deleteBegin, insEnd))
        {
            if (deleteInst->get_instr_type() != Instruction::OpID::alloca)
                entryBB->get_instructions().remove(deleteInst);
        }
        // 处理关系并添加BR指令
        for (auto &&succ : modifyedBB)
        {
            entryBB->remove_succ_basic_block(succ);
        }
        BranchInst::create_br(entryTail, entryBB);

        // 处理后继块之间关系
        for (auto &&succ : modifyedBB)
        {
            succ->remove_pre_basic_block(entryBB);
            succ->add_pre_basic_block(entryTail);
        }

        for (auto &&[workBB, storeIns] : workList)
        {
            auto callIns = dynamic_cast<CallInst *>(storeIns->get_rval());
            // 删除跳转指令
            workBB->delete_instr(workBB->get_terminator());
            auto retbb = *(workBB->get_succ_basic_blocks().begin());
            workBB->remove_succ_basic_block(retbb);
            retbb->remove_pre_basic_block(workBB);

            int i = 1;
            auto operands = callIns->get_operands();
            for (auto &&argAlloc : paramsAlloc)
            {
                auto alloc = operands[i++];
                StoreInst::create_store(alloc, argAlloc, workBB);
            }
            workBB->delete_instr(callIns);
            workBB->delete_instr(storeIns);
            BranchInst::create_br(entryTail, workBB);
        }
    }
}