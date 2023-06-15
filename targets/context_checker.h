#ifndef __MML_TARGETS_CONTEXT_CHECKER_H__
#define __MML_TARGETS_CONTEXT_CHECKER_H__

#include "targets/basic_ast_visitor.h"

namespace mml {

  class context_checker: public basic_ast_visitor {
    bool _returnSeen;
    bool _stopOrNextSeen;
    int _cycleDepth;

  public:
    context_checker(std::shared_ptr<cdk::compiler> compiler) :
        basic_ast_visitor(compiler), _returnSeen(false), _stopOrNextSeen(false), _cycleDepth(0) {
    }

  public:
    ~context_checker() {
      os().flush();
    }

  public:
    inline void setReturnSeen(bool returnSeen) {
      _returnSeen = returnSeen;
    }

    inline bool returnSeen() {
      return _returnSeen;
    }

  public:
    inline void setStopOrNextSeen(bool stopOrNextSeen) {
      _stopOrNextSeen = stopOrNextSeen;
    }

    inline bool stopOrNextSeen() {
      return _stopOrNextSeen;
    }

  public:
    inline void setCycleDepth(int cycleDepth) {
      _cycleDepth = cycleDepth;
    }

    inline int cycleDepth() {
      return _cycleDepth;
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
