#ifndef __MML_AST_function_H__
#define __MML_AST_function_H__

#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>
#include "ast/block_node.h"

namespace mml {

  class function_node: public cdk::typed_node {
    cdk::sequence_node *_arguments;
    mml::block_node *_block;

  public:
    function_node(int lineno, cdk::sequence_node *arguments, mml::block_node *block) :
        cdk::typed_node(lineno), _arguments(arguments), _block(block) {
      type(cdk::primitive_type::create(0, cdk::TYPE_VOID));
    }

    function_node(int lineno, std::shared_ptr<cdk::basic_type> funType, cdk::sequence_node *arguments, mml::block_node *block) :
        cdk::typed_node(lineno), _arguments(arguments), _block(block) {
      type(funType);
    }

  public:
    cdk::sequence_node* arguments() {
      return _arguments;
    }
    cdk::typed_node* argument(size_t ax) {
      return dynamic_cast<cdk::typed_node*>(_arguments->node(ax));
    }
    mml::block_node* block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_node(this, level);
    }

  };

} // mml

#endif
