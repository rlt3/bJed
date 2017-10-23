//
//  CLI.hpp
//  bjou++
//
//  Created by Brandon Kammerdiener on 1/8/17.
//  Copyright © 2017 me. All rights reserved.
//

#ifndef CLI_hpp
#define CLI_hpp

#include "Compile.hpp"
#include "Context.hpp"
#include "Global.hpp"

#ifdef BJOU_DEBUG_BUILD
#define SAVE_BJOU_DEBUG_BUILD
#endif
#undef BJOU_DEBUG_BUILD
#include "tclap/StdOutput.h"
#ifdef SAVE_BJOU_DEBUG_BUILD
#define BJOU_DEBUG_BUILD
#endif

#include "rlutil.h"

#include <mutex>

// sorry..
#ifdef BJOU_USE_COLOR
#define bjouSetColor(COLOR) rlutil::setColor(COLOR)
#define bjouSetBackgroundColor(COLOR) rlutil::setBackgroundColor(COLOR)
#define bjouResetColor() rlutil::resetColor()
#else
#define bjouSetColor(COLOR) ;
#define bjouSetBackgroundColor(COLOR) ;
#define bjouResetColor() ;
#endif

class bJouOutput : public TCLAP::StdOutput {
  public:
    bJouOutput();

    virtual void failure(TCLAP::CmdLineInterface & c, TCLAP::ArgException & e);
    virtual void usage(TCLAP::CmdLineInterface & c);
    virtual void version(TCLAP::CmdLineInterface & c);
};

namespace TCLAP {
class SwitchArg;
template <class T> class ValueArg;
template <class T> class MultiArg;
template <class T> class UnlabeledMultiArg;
} // namespace TCLAP

namespace bjou {
struct Parser;

struct TCLAPArgSet {
    TCLAP::SwitchArg & verbose_arg;
    TCLAP::SwitchArg & justcheck_arg;
    TCLAP::SwitchArg & time_arg;
    TCLAP::SwitchArg & symbols_arg;
    TCLAP::SwitchArg & noparallel_arg;
    TCLAP::SwitchArg & opt_arg;
    TCLAP::SwitchArg & module_arg;
    TCLAP::MultiArg<std::string> & module_search_path_arg;
    TCLAP::SwitchArg & nopreload_arg;
    TCLAP::SwitchArg & lld_arg;
    TCLAP::ValueArg<std::string> & output_arg;
    TCLAP::MultiArg<std::string> & link_arg;
    TCLAP::UnlabeledMultiArg<std::string> & files;
};

using namespace rlutil;

void prettyPrintTimeMaj(milliseconds ms, std::string label);
void prettyPrintTimeMin(milliseconds ms, std::string label);

void internalError(std::string message);

void _more(std::string message);

template <typename... continuations>
void _more(std::string message, continuations... c) {
    std::cout << "        *** ";
    bjouSetColor(GREEN);
    std::cout << message;
    bjouResetColor();
    std::cout << "\n";
    _more(c...);
}

template <typename... continuations>
void _error(Context & context, std::string message) {
    bjouSetColor(CYAN);
    std::cout << "bJou :: " << context.filename << " :: " << context.begin.line
              << " :: " << context.begin.character << "\n";
    bjouSetColor(RED);
    std::cout << "     Error: ";
    bjouSetColor(GREEN);
    std::cout << message;
    bjouResetColor();
    std::cout << "\n";
}

template <typename... continuations>
void _warning(Context & context, std::string message, continuations... c) {
    bjouSetColor(CYAN);
    std::cout << "bJou :: " << context.filename << " :: " << context.begin.line
              << " :: " << context.begin.character << "\n";
    bjouSetColor(YELLOW);
    std::cout << "   Warning: ";
    bjouSetColor(GREEN);
    std::cout << message;
    bjouResetColor();
    std::cout << "\n";
}

std::string linenobuf(int ln, bool mark = false);
void _here(Context & context);

void error(Context context, std::string message, bool exit = true);
void error(Context context, std::string message,
           std::vector<std::string> continuations);

template <typename... continuations>
void error(Context context, std::string message, bool exit,
           continuations... c) {
    std::lock_guard<std::mutex> lock(cli_mtx);
    _error(context, message);
    _more(c...);
    if (exit)
        compilation->abort();
}

void errorl(Context context, std::string message, bool exit = true);
void errorl(Context context, std::string message, bool exit,
            std::vector<std::string> continuations);

template <typename... continuations>
void errorl(Context context, std::string message, bool exit,
            continuations... c) {
    std::lock_guard<std::mutex> lock(cli_mtx);
    _error(context, message);
    _more(c...);
    _here(context);
    if (exit)
        compilation->abort();
}

Context errornextGetContext(Parser & parser);

void errornext(Parser & parser, std::string message, bool exit = true);
void errornext(Parser & parser, std::string message, bool exit,
               std::vector<std::string> continuations);

template <typename... continuations>
void errornext(Parser & parser, std::string message, bool exit,
               continuations... c) {
    Context e_context = errornextGetContext(parser);
    errorl(e_context, message, true, c...);
}

void warning(Context context, std::string message);
void warning(Context context, std::string message,
             std::vector<std::string> continuations);

template <typename... continuations>
void warning(Context context, std::string message, continuations... c) {
    std::lock_guard<std::mutex> lock(cli_mtx);
    _warning(context, message);
    _more(c...);
}

void warningl(Context context, std::string message);
void warningl(Context context, std::string message,
              std::vector<std::string> continuations);

template <typename... continuations>
void warningl(Context context, std::string message, continuations... c) {
    std::lock_guard<std::mutex> lock(cli_mtx);
    _warning(context, message);
    _more(c...);
    _here(context);
}
} // namespace bjou

#endif /* CLI_hpp */
