#pragma once

#include "Pass.h"
#include "Module.h"

class tailCall : public Pass
{
public:
    using Pass::Pass;
    void execute() final;
    const std::string get_name() const override { return name; }

private:
    const std::string name = "tailCall";
};