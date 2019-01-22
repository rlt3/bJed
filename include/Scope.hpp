//
//  Scope.hpp
//  bjou++
//
//  Created by Brandon Kammerdiener on 1/24/17.
//  Copyright © 2017 me. All rights reserved.
//

#ifndef Scope_hpp
#define Scope_hpp

#include "ASTNode.hpp"
#include "Maybe.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace bjou {
struct Symbol;
template <typename T> struct _Symbol;
struct SymbolNamespace;
struct ASTNode;
struct Context;

struct Scope {
    Scope(std::string description, Scope * _parent);
    ~Scope();

    std::string description;
    Scope * parent;
    std::unordered_map<std::string, Symbol *> symbols;
    std::vector<Scope *> scopes;

    Maybe<Symbol *> getSymbol(Scope * startingScope, ASTNode * _identifier,
                              Context * context = nullptr, bool traverse = true,
                              bool fail = true, bool checkUninit = true,
                              bool countAsReference = true);
    Maybe<Symbol *> getSymbol(Scope * startingScope,
                              std::string & qualifiedIdentifier,
                              Context * context = nullptr, bool traverse = true,
                              bool fail = true, bool checkUninit = true,
                              bool countAsReference = true);
    void addSymbol(Symbol * symbol, Context * context);
    void addSymbol(_Symbol<Procedure> * symbol, Context * context);
    void addSymbol(_Symbol<TemplateProc> * symbol, Context * context);
    void addProcSymbol(Symbol * symbol, bool is_extern, Context * context);
    void printSymbols(int indent) const;
};

void printSymbolTables();
} // namespace bjou

#endif /* Scope_hpp */
