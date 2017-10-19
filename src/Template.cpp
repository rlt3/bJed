//
//  Template.cpp
//  bjou++
//
//  Created by Brandon Kammerdiener on 1/24/17.
//  Copyright © 2017 me. All rights reserved.
//

#include "Template.hpp"
#include "Symbol.hpp"
#include "FrontEnd.hpp"

namespace bjou {
    static void checkTypeTemplateInstantiation(ASTNode * _defs, ASTNode * _inst) {
        TemplateDefineList * defs = (TemplateDefineList*)_defs;
        TemplateInstantiation * inst = (TemplateInstantiation*)_inst;
        
        if (defs->getElements().size() != inst->getElements().size()) {
            errorl(inst->getContext(), "Number of tempalate arguments does not match template definition.", false, "expected " + std::to_string(defs->getElements().size()) + ", got " + std::to_string(inst->getElements().size()));
            errorl(defs->getContext(), "Template definition:");
        }
        for (int idx = 0; idx < (int)defs->getElements().size(); idx += 1) {
            ASTNode * d = defs->getElements()[idx];
            ASTNode * i = inst->getElements()[idx];
            
            i->analyze();
            switch (d->nodeKind) {
                case ASTNode::TEMPLATE_DEFINE_TYPE_DESCRIPTOR:
                    if (!IS_DECLARATOR(i)) {
                        if (IS_EXPRESSION(i))
                            errorl(i->getContext(), "Template definition calls for type declarator.", false, "got expression");
                        else errorl(i->getContext(), "Template definition calls for type declarator.", false);
                        errorl(d->getContext(), "Type declarator specified here:");
                    }
                    break;
                case ASTNode::TEMPLATE_DEFINE_VARIADIC_TYPE_ARGS:
                    // @incomplete
                    BJOU_DEBUG_ASSERT(false);
                    break;
                case ASTNode::TEMPLATE_DEFINE_EXPRESSION:
                    // @incomplete
                    BJOU_DEBUG_ASSERT(false);
                    break;
                    
                default:
                    internalError("invalid node in template define list");
            }
        }
    }
    
    static bool terminalShouldReplace(ASTNode * term, ASTNode * d) {
        TemplateDefineElement * elem = (TemplateDefineElement*)d;
        if (IS_DECLARATOR(term)) {
            Declarator * decl = (Declarator*)term;
            Identifier * ident = (Identifier*)decl->getIdentifier();
            if (ident->getUnqualified() == elem->getName())
                return true;
        } else if (IS_EXPRESSION(term)) {
            // @incomplete
        }
        return false;
    }
    
    static void templateReplaceTerminals(ASTNode * _template, ASTNode * _def, ASTNode * _inst) {
        TemplateDefineList * def = (TemplateDefineList*)_def;
        TemplateInstantiation * inst = (TemplateInstantiation*)_inst;
        
        std::vector<ASTNode*> terminals;
        _template->unwrap(terminals);
        
        BJOU_DEBUG_ASSERT(def->getElements().size() == inst->getElements().size());
        
        for (ASTNode * term : terminals) {
            for (int idx = 0; idx < (int)def->getElements().size(); idx += 1) {
                ASTNode * d = def->getElements()[idx];
                ASTNode * i = inst->getElements()[idx];
                ASTNode * i_clone = i->clone();
                i_clone->setContext(term->getContext());
                if (terminalShouldReplace(term, d))
                    (*term->replace)(term->parent, term, i_clone);
            }
        }
    }
    
    static void templateReplaceTerminals(std::vector<ASTNode*>& terminals, ASTNode * _def, ASTNode * _inst) {
        TemplateDefineList * def = (TemplateDefineList*)_def;
        TemplateInstantiation * inst = (TemplateInstantiation*)_inst;
        
        BJOU_DEBUG_ASSERT(def->getElements().size() == inst->getElements().size());
        
        for (ASTNode * term : terminals) {
            for (int idx = 0; idx < (int)def->getElements().size(); idx += 1) {
                ASTNode * d = def->getElements()[idx];
                ASTNode * i = inst->getElements()[idx];
                if (terminalShouldReplace(term, d))
                    (*term->replace)(term->parent, term, i->clone());
            }
        }
    }
    
    Struct * makeTemplateStruct(ASTNode * _ttype, ASTNode * _inst) {
        TemplateStruct * ttype = (TemplateStruct*)_ttype;
        Struct * s = (Struct*)ttype->_template;
        Scope * scope = ttype->getScope();
        TemplateInstantiation * inst = (TemplateInstantiation*)_inst;
        
        checkTypeTemplateInstantiation(ttype->getTemplateDef(), inst);
        
        _Symbol<Struct> * newsym = new _Symbol<Struct>(s->getName(), s, inst);
        std::string mangledName = newsym->mangledString(scope);
        
        Maybe<Symbol*> m_sym = scope->getSymbol(scope, mangledName, nullptr, true, false, false);
        Symbol * sym = nullptr;
        if (m_sym.assignTo(sym)) {
            // found it
            BJOU_DEBUG_ASSERT(sym->isType());
            BJOU_DEBUG_ASSERT(sym->node()->nodeKind == ASTNode::STRUCT);
            return (Struct*)sym->node();
        }
        
        Struct * clone = (Struct*)s->clone();
        clone->setName(mangledName);
        clone->inst = inst;
        
        clone->preDeclare(scope);
        clone->addSymbols(scope);
        
        templateReplaceTerminals(clone, ttype->getTemplateDef(), inst);
        
        ((StructType*)clone->getType())->complete();
        
        clone->analyze(true);
        compilation->frontEnd.deferredAST.push_back(clone);
        
        return clone;
    }
    
    static ASTNode * replacement_policy_isolated_params(ASTNode * parent, ASTNode * old_node, ASTNode * new_node) {
        std::vector<ASTNode*> * terminals = (std::vector<ASTNode*>*)parent;
        
        for (ASTNode * &node : *terminals)
            if (node == old_node)
                node = new_node;
        
        return new_node;
    }
  
	enum patternMatchTypeStep {
		IDENTITY,
		ARRAY,
		DYNAMIC_ARRAY,
		POINTER,
		MAYBE,
		PROCEDURE,
		TEMPLATE_INSTANTIATION
	};

	static void patternMatchType(const Type * pattern, const Type * subject) {
		if (pattern->getBase() == pattern) {
		} else {
			switch (pattern->kind) {
				case Type::INVALID:
				case Type::PLACEHOLDER:
				case Type::PRIMATIVE:
				case Type::BASE:
				case Type::STRUCT:
				case Type::ENUM:
				case Type::ALIAS:
				case Type::ARRAY:
				case Type::DYNAMIC_ARRAY:
				case Type::POINTER:
				case Type::MAYBE:
				case Type::TUPLE:
				case Type::PROCEDURE:
				case Type::TEMPLATE_STRUCT:
				case Type::TEMPLATE_ALIAS:
					return;
			}
		}
	}

    bool checkTemplateProcInstantiation(ASTNode * _tproc, ASTNode * _passed_args, ASTNode * _inst, Context * context, bool fail, TemplateInstantiation * new_inst) {
        bool delete_new_inst = false;
        TemplateProc * tproc = (TemplateProc*)_tproc;
        TemplateDefineList * def = (TemplateDefineList*)tproc->getTemplateDef();
        Procedure * proc = (Procedure*)tproc->_template;
        ArgList * passed_args = (ArgList*)_passed_args;
        TemplateInstantiation * inst = (TemplateInstantiation*)_inst;
        
        proc->desugarThis();
        
        std::vector<ASTNode*>& params = proc->getParamVarDeclarations();
        
        std::vector<std::pair<TemplateDefineElement*, Declarator*> > checklist;
        
        for (auto elem : def->getElements())
            checklist.push_back(std::make_pair((TemplateDefineElement*)elem, nullptr));
        
        if (inst) {
            if (inst->getElements().size() > checklist.size()) {
                if (fail)
                    errorl(inst->getContext(), "Too many template arguments."); // @bad error message
                else return false;
            }
            for (int i = 0; i < (int)checklist.size(); i += 1)
                checklist[i].second = (Declarator*)inst->getElements()[i];
        }
        
        if (passed_args) {
            if (passed_args->getExpressions().size() != params.size())
                return false;
            
            for (int i = 0; i < (int)params.size(); i += 1) {
                Declarator * param_decl = (Declarator*)((VariableDeclaration*)params[i])->getTypeDeclarator();
                Declarator * param_base_decl = (Declarator*)param_decl->getBase(); // @leak
                param_base_decl->setScope(param_decl->getScope());
                const Type * passed_type = passed_args->getExpressions()[i]->getType();
                Declarator * passed_decl = passed_type->getGenericDeclarator();
                BJOU_DEBUG_ASSERT(passed_decl);
                passed_decl->setScope(passed_args->getExpressions()[i]->getScope());
                Declarator * passed_base_decl = (Declarator*)passed_decl->getBase();
                passed_base_decl->setScope(passed_decl->getScope());
                const Type * passed_base_t = passed_base_decl->getType()->getOriginal();
                
                for (auto& check : checklist) {
                    if (!check.second) {
                        if (check.first->name == param_decl->mangleSymbol()) {
                            check.second = passed_decl;
                        } else if (check.first->name == param_base_decl->mangleSymbol()) {
                            check.second = passed_base_decl;
                        } else if (passed_base_t->isStruct()) { // sorry about casting away the constness..
                            const StructType * passed_struct_t = (const StructType*)passed_base_t;
                            if (param_base_decl->getTemplateInst()) {
                                TemplateInstantiation * param_base_inst = (TemplateInstantiation*)param_base_decl->getTemplateInst();
                                for (int j  = 0; j < (int)param_base_inst->getElements().size(); j += 1) {
                                    ASTNode * elem = param_base_inst->getElements()[j];
                                    if (IS_DECLARATOR(elem)) {
                                        Declarator * e_decl = (Declarator*)elem;
                                        Declarator * e_base_decl = (Declarator*)e_decl->getBase();
                                        if (check.first->name == e_base_decl->mangleSymbol()) {
                                            if (!passed_struct_t->inst) {
                                                if (fail) {
                                                    errorl(passed_args->getExpressions()[i]->getContext(), "Template procedure expecting a through template argument type being passed a non-template argument.", false);
                                                    errorl(params[i]->getContext(), "Through-template argument defined here.");
                                                } else return false;
                                            }
                                            check.second = (Declarator*)passed_struct_t->inst->getElements()[j];
                                            break;
                                        }
                                    } /* else if (elem->nodeKind == ASTNode::IDENTIFIER) {
                                       Identifier * e_ident = (Identifier*)elem;
                                       if (check.first->name == e_ident->getUnqualified()) {
                                       // ...
                                       }
                                       }*/
                                }
                            }
                        }
                    }
                }
            }
        }
        
        if (!new_inst) {
            new_inst = new TemplateInstantiation;
            delete_new_inst = true;
        }
        
        std::vector<std::string> incomplete_errs;
        for (auto& check : checklist) {
            if (check.second) {
                if (new_inst)
                    new_inst->addElement(check.second->clone());
            } else incomplete_errs.push_back(check.first->getName());
        }
        if (!incomplete_errs.empty()) {
            if (delete_new_inst)
                delete new_inst;
            if (fail)
                errorl(*context, "Could not complete the following templates in '" + proc->getName() + "':", true, incomplete_errs);
            else return false;
        }
        
        if (passed_args) {
            std::vector<const Type*> arg_types;
            for (ASTNode * expr : passed_args->getExpressions())
                arg_types.push_back(expr->getType());
            std::vector<const Type*> new_param_types;
            
            std::vector<VariableDeclaration*> var_clones;
            std::vector<ASTNode*> terms;
            for (ASTNode * _var : params) {
                VariableDeclaration *var_clone, * var = (VariableDeclaration*)_var;
                var_clone = (VariableDeclaration*)var->clone();
                var_clone->getTypeDeclarator()->addSymbols(tproc->getScope());
                var_clone->unwrap(terms);
                var_clones.push_back(var_clone);
            }
            templateReplaceTerminals(terms, def, new_inst);
            
            for (ASTNode * _var : var_clones) {
                VariableDeclaration * var = (VariableDeclaration*)_var;
                
                // We need to analyze and THEN get the type, making sure to access
                // the declarator via getTypeDeclarator() because if the declarator
                // is a template, it will be replaced.
                var->getTypeDeclarator()->analyze(true);
                new_param_types.push_back(var->getTypeDeclarator()->getType());
                delete _var;
            }
            
            for (int i = 0; i < (int)arg_types.size(); i += 1) {
                if (!arg_types[i]->equivalent(new_param_types[i])) {
                    if (delete_new_inst)
                        delete new_inst;
                    return false;
                }
            }
        }
        
        if (delete_new_inst)
            delete new_inst;
        
        return true;
    }
    
    Procedure * makeTemplateProc(ASTNode * _tproc, ASTNode * _passed_args, ASTNode * _inst, Context * context, bool fail) {
        TemplateProc * tproc = (TemplateProc*)_tproc;
        Scope * scope = tproc->getScope();
        Procedure * proc = (Procedure*)tproc->_template;
        TemplateDefineList * def = (TemplateDefineList*)tproc->getTemplateDef();
        
        TemplateInstantiation * new_inst = new TemplateInstantiation;
        checkTemplateProcInstantiation(_tproc, _passed_args, _inst, context, fail, new_inst);
        
        std::vector<ASTNode*> save_params, new_params, terms;
        for (ASTNode * param : proc->getParamVarDeclarations()) {
            save_params.push_back(param);
            
            ASTNode * clone = param->clone();
            new_params.push_back(clone);
            clone->unwrap(terms);
        }
        
        proc->getParamVarDeclarations().clear();
        
        templateReplaceTerminals(terms, def, new_inst);
        
        for (ASTNode * new_param : new_params)
            proc->addParamVarDeclaration(new_param);
        
        _Symbol<Procedure> * newsym = new _Symbol<Procedure>(proc->getName(), proc, new_inst);
        std::string mangledName = newsym->mangledString(scope);
        
        Maybe<Symbol*> m_sym = scope->getSymbol(scope, mangledName, nullptr, true, false, false);
        Symbol * sym = nullptr;
        if (m_sym.assignTo(sym)) {
            // found it
            BJOU_DEBUG_ASSERT(sym->isProc());
            BJOU_DEBUG_ASSERT(sym->node()->nodeKind == ASTNode::PROCEDURE);
            return (Procedure*)sym->node();
        }
        
        proc->getParamVarDeclarations().clear();
        for (ASTNode * param : save_params)
            proc->addParamVarDeclaration(param);
        
        for (ASTNode * new_param : new_params)
            delete new_param;
        
        Procedure * clone = (Procedure*)proc->clone();
        clone->setName(proc->getName());// mangledName);
        clone->inst = new_inst;
        
        templateReplaceTerminals(clone, tproc->getTemplateDef(), new_inst);
        
        if (tproc->getFlag(TemplateProc::FROM_THROUGH_TEMPLATE))
            clone->setFlag(ASTNode::SYMBOL_OVERWRITE, true);
        clone->addSymbols(scope);
        
        clone->analyze(true);
        compilation->frontEnd.deferredAST.push_back(clone);
        
        return clone;
    }
}
