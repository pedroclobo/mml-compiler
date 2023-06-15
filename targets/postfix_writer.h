#ifndef __MML_TARGETS_POSTFIX_WRITER_H__
#define __MML_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"
#include "targets/context.h"

#include <sstream>
#include <vector>
#include <stack>
#include <set>
#include <cdk/emitters/basic_postfix_emitter.h>

namespace mml {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<mml::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;

    std::vector<int> _whileCond, _whileEnd;

    std::stack<std::string> _textLabels; // text label of current function
    std::stack<std::string> _returnLabels; // label of current function return
    std::stack<bool> _returnSeen;
    std::set<std::string> _foreignFunctions;
    std::stack<int> _offsets;

    std::stack<mml::context_type> _contexts;

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<mml::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0) {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

  public:
    inline void pushCondLabel(int label) {
      _whileCond.push_back(label);
    }

    inline void popCondLabel() {
      _whileCond.pop_back();
    }

    inline int condLabel(int level) {
      return _whileCond[_whileCond.size() - level];
    }

  public:
    inline void pushEndLabel(int label) {
      _whileEnd.push_back(label);
    }

    inline void popEndLabel() {
      _whileEnd.pop_back();
    }

    inline int endLabel(int level) {
      return _whileEnd[_whileEnd.size() - level];
    }

  public:
    inline void pushTextLabel(std::string textLabel) {
      _textLabels.push(textLabel);
    }

    inline void popTextLabel() {
      _textLabels.pop();
    }

    inline std::string textLabel() {
      if (_textLabels.empty()) {
        return "";
      }
      return _textLabels.top();
    }

  public:
    inline void pushReturnLabel(std::string returnLabel) {
      _returnLabels.push(returnLabel);
    }

    inline void popReturnLabel() {
      _returnLabels.pop();
    }

    inline std::string returnLabel() {
      return _returnLabels.top();
    }

  public:
    inline void pushReturnSeen() {
      _returnSeen.push(false);
    }

    inline void popReturnSeen() {
      _returnSeen.pop();
    }

    inline void setReturnSeen(bool returnSeen) {
      _returnSeen.top() = returnSeen;
    }

    inline bool returnSeen() {
      return _returnSeen.top();
    }

  public:
    inline void addForeignFunction(std::string identifier) {
      _foreignFunctions.insert(identifier);
    }

    inline std::set<std::string> foreignFunctions() {
      return _foreignFunctions;
    }

  public:
    inline void pushOffset(int offset) {
      _offsets.push(offset);
    }

    inline void popOffset() {
      _offsets.pop();
    }

    inline void setOffset(int offset) {
      _offsets.top() = offset;
    }

    inline int offset() {
      return _offsets.top();
    }

  public:
    inline void pushContext(mml::context_type context) {
      _contexts.push(context);
    }

    inline void popContext() {
      _contexts.pop();
    }

    inline mml::context_type context() {
      return _contexts.top();
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // mml

#endif
