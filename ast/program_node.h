#ifndef __MML_AST_PROGRAM_NODE_H__
#define __MML_AST_PROGRAM_NODE_H__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/sequence_node.h>

namespace mml {

  /**
   * Class for describing program nodes.
   */
  class program_node: public cdk::basic_node {
    cdk::sequence_node *_outerdeclarations;
    cdk::sequence_node *_innerdeclarations;
    cdk::sequence_node *_instructions;

  public:
    inline program_node(int lineno, cdk::sequence_node *outerdeclarations, cdk::sequence_node *innerdeclarations, cdk::sequence_node *instructions) :
        cdk::basic_node(lineno), _outerdeclarations(outerdeclarations), _innerdeclarations(innerdeclarations), _instructions(instructions) {
    }

  public:
    inline cdk::sequence_node *outerdeclarations() {
      return _outerdeclarations;
    }

    inline cdk::sequence_node *innerdeclarations() {
      return _innerdeclarations;
    }

    inline cdk::sequence_node *instructions() {
      return _instructions;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_program_node(this, level);
    }

  };

} // mml

#endif
