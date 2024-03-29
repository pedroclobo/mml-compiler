#ifndef __MML_AST_FUNCTION_DEFINITION_NODE_H__
#define __MML_AST_FUNCTION_DEFINITION_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include "ast/block_node.h"

namespace mml {

  class function_definition_node: public cdk::expression_node {
    cdk::sequence_node *_arguments;
    mml::block_node *_block;

  public:
    inline function_definition_node(int lineno, cdk::sequence_node *arguments, std::shared_ptr<cdk::basic_type> retType, mml::block_node *block) :
        cdk::expression_node(lineno), _arguments(arguments), _block(block) {
      std::vector<std::shared_ptr<cdk::basic_type>> input;
      for (size_t i = 0; i < arguments->size(); i++) {
        input.push_back(argument(i)->type());
      }
      type(cdk::functional_type::create(input, retType));
    }

  public:
    inline cdk::sequence_node* arguments() {
      return _arguments;
    }
    inline cdk::typed_node* argument(size_t ax) {
      return dynamic_cast<cdk::typed_node*>(_arguments->node(ax));
    }
    inline mml::block_node* block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }

  };

} // mml

#endif
