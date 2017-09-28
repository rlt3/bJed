//
//  Misc.hpp
//  bjou++
//
//  Created by Brandon Kammerdiener on 1/8/17.
//  Copyright © 2017 me. All rights reserved.
//

#ifndef Misc_h
#define Misc_h

#include "Context.hpp"

#include <algorithm>
#include <assert.h>
#include <cstring>
#include <future>

#define B_MAX(a, b) ((a) > (b) ? (a) : (b))

template <size_t N>
bool s_in_a(const char * s, const char * (&a)[N]) {
    for (auto elem : a)
        if (std::strcmp(elem, s) == 0)
            return true;
    return false;
}

namespace bjou {
    template <typename F, typename R>
    inline std::future<R> runasync(F& f) {
        return std::async(std::launch::async, f);
    }
    
    template <typename F, typename... Ts, typename R>
    inline std::future<R> runasync(F& f, Ts&... params) {
        return std::async(std::launch::async, std::forward<F>(f), std::forward<Ts>(params)...);
    }
}

std::string de_quote(std::string& str);
std::string str_escape(std::string& str);
char get_ch_value(std::string& str);

#ifdef BJOU_DEBUG_BUILD
#include <string>
namespace bjou {
    void internalError(std::string message);
    void errorl(Context context, std::string message, bool exit);
}
inline void _BJOU_TRIGGER_DEBUG_ASSERT(int line, const char* file) {
    bjou::Context context;
    context.filename = file;
    context.begin.line = context.end.line = line;
    context.begin.character = context.end.character = 1;
    bjou::errorl(context, "assertion failed on line " + std::to_string(line) + " of " + std::string(file), false);
    bjou::internalError("exiting due to a failed assertion");
}
#define BJOU_DEBUG_ASSERT(expr)                 \
    if (!(expr))                                \
        _BJOU_TRIGGER_DEBUG_ASSERT(__LINE__, __FILE__)
#else
    #define BJOU_DEBUG_ASSERT(expr) ;
#endif




// I really hate the C preprocessor.. let's do something about that.
#define EVAL(x) x
#define _STR(x) #x
#define STR(x) _STR(x)
#define _CAT3(x, y, z) x##y##z
#define CAT3(x, y, z) _CAT3(x, y, z)

#define BJOU_VER_MAJ 0
#define BJOU_VER_MIN 6
#define BJOU_VER CAT3(BJOU_VER_MAJ, ., BJOU_VER_MIN)
#define BJOU_VER_STR STR(BJOU_VER)

#define BJOU_VER_COLOR GREEN

#define _BJOU_VER_COLOR_STR STR(BJOU_VER_COLOR)
#define BJOU_VER_COLOR_STR _BJOU_VER_COLOR_STR

#endif /* Misc_h */
