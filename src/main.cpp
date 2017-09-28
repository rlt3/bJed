//
//  main.cpp
//  bjou++
//
//  Created by Brandon Kammerdiener on 1/4/17.
//  Copyright © 2017 me. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <bitset>
#include <mutex>

#include "ASTNode.hpp"
#include "Global.hpp"
#include "Compile.hpp"
#include "FrontEnd.hpp"
#include "BackEnd.hpp"
#include "LLVMBackEnd.hpp"
#include "CLI.hpp"
#include "Misc.hpp"

#ifdef BJOU_DEBUG_BUILD
#define SAVE_BJOU_DEBUG_BUILD
#endif
#undef BJOU_DEBUG_BUILD
#include "tclap/CmdLine.h"
#ifdef SAVE_BJOU_DEBUG_BUILD
#define BJOU_DEBUG_BUILD
#endif

namespace bjou { struct ASTNode; Compilation * compilation = nullptr; }
std::mutex cli_mtx;

int main(int argc, const char ** argv) {
    // below are the command line options that this compiler takes.
    TCLAP::CmdLine cmd_line("bJou\nA friendly language and compiler written by Brandon Kammerdiener", ' ', BJOU_VER_STR);
    bJouOutput output;
    cmd_line.setOutput(&output);
    
    TCLAP::SwitchArg verbose_arg("v", "verbose", "Print the LLVM IR to STDOUT.", cmd_line);
    TCLAP::SwitchArg justcheck_arg("", "justcheck", "Verify source correctness, but skip optimization and codegen.", cmd_line);
    TCLAP::SwitchArg time_arg("", "time", "Print the running times of compilation stages to STDOUT.", cmd_line);
    TCLAP::SwitchArg symbols_arg("", "symbols", "Print symbol tables to STDOUT.", cmd_line);
    TCLAP::SwitchArg noparallel_arg("", "noparallel", "Turn compilation parallelization off.", cmd_line);
    TCLAP::SwitchArg opt_arg("O", "optimize", "Run LLVM optimization passes.", cmd_line);
    TCLAP::SwitchArg module_arg("m", "module", "Create a module file instead of an executable.", cmd_line);
    TCLAP::SwitchArg nopreload_arg("", "nopreload", "Do not automatically import preload modules.", cmd_line);
    TCLAP::ValueArg<std::string> output_arg("o", "output", "Name of target output file.", false, "", "file name", cmd_line);
    TCLAP::MultiArg<std::string> link_arg("l", "link", "Name of libraries to link.", false, "library name", cmd_line);
    TCLAP::UnlabeledMultiArg<std::string> files("files", "Input source files.", false, "file name(s)", cmd_line); // this actually is required, but I wanted to provide the error message instead of tclap
    
    bjou::TCLAPArgSet args = { verbose_arg, justcheck_arg, time_arg, symbols_arg, noparallel_arg, opt_arg, module_arg, nopreload_arg, output_arg, link_arg, files };
    
    cmd_line.parse(argc, (char**)argv); // cast away constness
    // end command line options
    
    bjou::FrontEnd frontEnd;
    bjou::LLVMBackEnd llvmBackEnd(frontEnd);
    
    compilation = new bjou::Compilation(frontEnd, llvmBackEnd, args);
    compilation->go();

    return 0;
}
