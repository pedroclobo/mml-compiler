#ifndef __MML_TARGETS_CONTEXT_CHECKER_H__
#define __MML_TARGETS_CONTEXT_CHECKER_H__

#include "targets/basic_ast_visitor.h"

namespace mml {

  class context_checker: public basic_ast_visitor {
    bool _return_seen;
    bool _stop_or_next_seen;
    int _cycle_depth;

  public:
    context_checker(std::shared_ptr<cdk::compiler> compiler) :
        basic_ast_visitor(compiler), _return_seen(false), _stop_or_next_seen(false), _cycle_depth(0) {
    }

  public:
    ~context_checker() {
      os().flush();
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
