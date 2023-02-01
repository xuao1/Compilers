#include <iostream>
#include "IRBuilder.h"
#include "SysYFDriver.h"
#include "SyntaxTreePrinter.h"
#include "ErrorReporter.h"
#include "SyntaxTreeChecker.h"
#include "Pass.h"
#include "DominateTree.h"
#include "Mem2Reg.h"
#include "ActiveVar.h"
#include "Check.h"
#include "tailCall.h"
#include "StrengthReduction.h"
#include "constPro.h"
#include "CodeExtraction.h"
#include "CSEin.h"
#include "CSEglo.h"


void print_help(const std::string& exe_name) {
  std::cout << "Usage: " << exe_name
            << " [ -h | --help ] [ -p | --trace_parsing ] [ -s | --trace_scanning ] [ -emit-ast ] [ -check ]"
            << " [ -emit-ir ] [ -O2 ] [ -O ] [ -av ] [ -o <output-file> ]"
            << " <input-file>"
            << std::endl;
}

int main(int argc, char *argv[])
{
    IRBuilder builder;  // 用于建立中间表示
    SysYFDriver driver;
    SyntaxTreePrinter printer;  // 用于输出语法树
    ErrorReporter reporter(std::cerr);  // 用于输出错误
    SyntaxTreeChecker checker(reporter);    // 检查语法错误

    bool print_ast = false;
    bool emit_ir = false;
    bool check = false;
    bool optimize_all = false;
    bool optimize = false;

    bool av = false;
    bool tc = false; //tailCall
    bool sr = false; //strengthReduction
    bool cp = false; //constPro: constant propagation
    bool ce = false; //codeExtraction
    bool cse_in = false; // CSEin
    bool cse_glo = false; // globalCSE

    std::string filename = "-";
    std::string output_llvm_file = "-";
    // 解析编译选项
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == std::string("-h") || argv[i] == std::string("--help")) {
            print_help(argv[0]);
            return 0;
        }
        else if (argv[i] == std::string("-p") || argv[i] == std::string("--trace_parsing")) {
            driver.trace_parsing = true;
        }
        else if (argv[i] == std::string("-s") || argv[i] == std::string("--trace_scanning")){
            driver.trace_scanning = true;
        }
        else if (argv[i] == std::string("-emit-ast")) {
            print_ast = true;
        }
        else if (argv[i] == std::string("-emit-ir")){
            emit_ir = true;
        }
        else if (argv[i] == std::string("-o")){
            output_llvm_file = argv[++i];
        }
        else if (argv[i] == std::string("-check")){
            check = true;
        }
        else if (argv[i] == std::string("-O2")){
            optimize_all = true;
            optimize = true;
        }
        else if (argv[i] == std::string("-O")){
            optimize = true;
        }
        else if(argv[i] == std::string("-av")){
            av = true;
        }
        else if(argv[i] == std::string("-tc")){
            tc = true;
        }
        else if(argv[i] == std::string("-sr")){
            sr = true;
        }
        else if(argv[i] == std::string("-cp")){
            cp = true;
        }
        else if(argv[i] == std::string("-ce")){
            ce = true;
        }
        else if(argv[i] == std::string("-cse-in")){
            cse_in = true;
        }
        else if(argv[i] == std::string("-cse")){
            cse_glo = true;
        }
        //  ...
        else {
            filename = argv[i];
        }
    }
    
    auto root = driver.parse(filename);
    if (print_ast)
        root->accept(printer);
    if (check)
        root->accept(checker);
    if (emit_ir) {  // 要输出中间表示
        root->accept(builder);
        auto m = builder.getModule();   // m 为 module 的指针 unique_ptr
        m->set_file_name(filename);
        m->set_print_name();
        if(optimize){
            PassMgr passmgr(m.get());   // m.get() 得到的是 Module 对象的指针

            if(optimize_all){           // 此处添加Mem2Reg（SSA化）前的优化
                // tailCall 依赖于位置信息获得变量信息，并且将会修改变量信息
                // 如后面的优化存在依赖于位置信息的优化需注意修改该 pass
                passmgr.addPass<tailCall>();
                //  ...
            }
            else {
                if(tc){
                    passmgr.addPass<tailCall>();
                }
                //  ...
            }

            passmgr.addPass<DominateTree>();
            passmgr.addPass<Mem2Reg>();

            if(optimize_all){           // 此处添加Mem2Reg（SSA化）后的优化
                passmgr.addPass<ActiveVar>();
                passmgr.addPass<StrengthReduction>();
                passmgr.addPass<constPro>();
                passmgr.addPass<CodeExtraction>();
                passmgr.addPass<CSEin>();
                passmgr.addPass<CSEglo>();
                //  ...
            }
            else {
                if(av){
                    passmgr.addPass<ActiveVar>();
                }
                if(sr){
                    passmgr.addPass<StrengthReduction>();
                }
                if(cp){
                    passmgr.addPass<constPro>();
                }
                if(ce){
                    passmgr.addPass<CodeExtraction>();
                }
                if(cse_in){
                    passmgr.addPass<CSEin>();
                }
                if(cse_glo){
                    passmgr.addPass<CSEglo>();
                }
                //  ...
            }
            passmgr.addPass<Check>();
            passmgr.execute();  // 执行扫描
            // 应该在这里加入检查中间表示的扫描
            m->set_print_name();
            // std::cout << "done" << std::endl;
            // Check IRChecker(m.get());
            // IRChecker.execute();
            // if(IRChecker.get_err_type()){   // 中间表示有问题
            //     std::cout << "err type" << IRChecker.get_err_type() << std::endl;
            //     IRChecker.print();
            //     return 0;
            // }
        }
        auto IR = m->print();
        if(output_llvm_file == "-"){
            std::cout << IR;
        }
        else {
            std::ofstream output_stream;
            output_stream.open(output_llvm_file, std::ios::out);
            output_stream << IR;    // 将中间表示的内容输出到文件内
            output_stream.close();
        }
    }
    return 0;
}
