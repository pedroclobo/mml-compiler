#ifndef __MML_TARGETS_POSTFIX_WRITER_H__
#define __MML_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <vector>
#include <stack>
#include <cdk/emitters/basic_postfix_emitter.h>

namespace mml {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<mml::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;

    bool _isGlobal = true;
    int _offset = 0; // FP offset
    std::vector<int> _whileCond, _whileEnd;

    std::stack<std::string> _textLabels; // text label of current function
    std::stack<std::shared_ptr<cdk::basic_type>> _functionTypes; // type of current function
    std::stack<std::string> _returnLabels; // label of current function return
    std::stack<bool> _returnSeen;

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
    inline void pushFunctionType(std::shared_ptr<cdk::basic_type> functionType) {
      _functionTypes.push(functionType);
    }

    inline void popFunctionType() {
      _functionTypes.pop();
    }

    inline std::shared_ptr<cdk::basic_type> functionType() {
      return _functionTypes.top();
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
