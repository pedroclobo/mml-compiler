#ifndef __MML_AST_FUNCTION_CALL_NODE_H__
#define __MML_AST_FUNCTION_CALL_NODE_H__

#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>

namespace mml {

  class function_call_node: public cdk::expression_node {
    cdk::expression_node *_function;
    cdk::sequence_node *_arguments;

  public:
    inline function_call_node(int lineno, cdk::expression_node *function, cdk::sequence_node *arguments) :
        cdk::expression_node(lineno), _function(function), _arguments(arguments) {
    }

  public:
    inline cdk::expression_node *function() {
      return _function;
    }
    inline cdk::sequence_node *arguments() {
      return _arguments;
    }
    void arguments(cdk::sequence_node *arguments) {
      _arguments = arguments;
    }
    inline cdk::expression_node* argument(size_t i) {
      return dynamic_cast<cdk::expression_node*>(_arguments->node(i));
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_call_node(this, level);
    }

  };

} // mml

#endif
