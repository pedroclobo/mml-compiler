#ifndef __MML_BASIC_AST_VISITOR_H__
#define __MML_BASIC_AST_VISITOR_H__

#include <string>
#include <memory>
#include <iostream>
#include <stack>
#include <cdk/compiler.h>
#include <cdk/symbol_table.h>
#include "targets/symbol.h"
#include "targets/context.h"

/* do not edit -- include node forward declarations */
#define __NODE_DECLARATIONS_ONLY__
#include ".auto/all_nodes.h"  // automatically generated
#undef __NODE_DECLARATIONS_ONLY__
/* do not edit -- end */

//!
//! Print nodes as XML elements to the output stream.
//!
class basic_ast_visitor {
protected:
  //! The owner compiler
  std::shared_ptr<cdk::compiler> _compiler;

private:

  // last symbol inserted in symbol table
  std::shared_ptr<mml::symbol> _new_symbol;
  std::stack<std::shared_ptr<cdk::basic_type>> _functionTypes; // type of current function
  std::stack<mml::context_type> _contexts;

protected:
  basic_ast_visitor(std::shared_ptr<cdk::compiler> compiler) :
      _compiler(compiler) {
  }

  bool debug() {
    return _compiler->debug();
  }

  std::ostream &os() {
    return *_compiler->ostream();
  }

public:
  virtual ~basic_ast_visitor() {
  }

public:
  std::shared_ptr<mml::symbol> new_symbol() {
    return _new_symbol;
  }

  void set_new_symbol(std::shared_ptr<mml::symbol> symbol) {
    _new_symbol = symbol;
  }

  void reset_new_symbol() {
    _new_symbol = nullptr;
  }

public:
  inline void pushFunctionType(std::shared_ptr<cdk::basic_type> functionType) {
    _functionTypes.push(functionType);
  }

  inline void popFunctionType() {
    _functionTypes.pop();
  }

  inline std::shared_ptr<cdk::basic_type> functionType() {
    if (_functionTypes.empty()) {
      return nullptr;
    }
    return _functionTypes.top();
  }

public:
  inline void pushContext(mml::context_type context) {
    _contexts.push(context);
  }

  inline void popContext() {
    _contexts.pop();
  }

  inline mml::context_type context() {
    if (_contexts.empty()) {
      return mml::CONTEXT_UNKNOWN;
    }
    return _contexts.top();
  }


public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#define __PURE_VIRTUAL_DECLARATIONS_ONLY__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __PURE_VIRTUAL_DECLARATIONS_ONLY__
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

};

#endif
