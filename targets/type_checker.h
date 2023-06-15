#ifndef __MML_TARGETS_TYPE_CHECKER_H__
#define __MML_TARGETS_TYPE_CHECKER_H__

#include "targets/basic_ast_visitor.h"
#include "targets/context.h"

namespace mml {

  /**
   * Print nodes as XML elements to the output stream.
   */
  class type_checker: public basic_ast_visitor {
    cdk::symbol_table<mml::symbol> &_symtab;

    basic_ast_visitor *_parent;

    std::shared_ptr<cdk::basic_type> _functionType;
    mml::context_type _context;

  public:
    type_checker(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<mml::symbol> &symtab, basic_ast_visitor *parent, std::shared_ptr<cdk::basic_type> functionType, mml::context_type context) :
        basic_ast_visitor(compiler), _symtab(symtab), _parent(parent), _functionType(functionType), _context(context) {
    }

  public:
    ~type_checker() {
      os().flush();
    }

  public:
    inline std::shared_ptr<cdk::basic_type> functionType() {
      return _functionType;
    }

  public:
    inline mml::context_type context() {
      return _context;
    }

  protected:
    void processUnaryExpression(cdk::unary_operation_node *const node, int lvl);
    void processBinaryExpression(cdk::binary_operation_node *const node, int lvl);
    template<typename T>
    void process_literal(cdk::literal_node<T> *const node, int lvl) {
    }

  public:
    // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
    // do not edit these lines: end

  };

} // mml

//---------------------------------------------------------------------------
//     HELPER MACRO FOR TYPE CHECKING
//---------------------------------------------------------------------------

#define CHECK_TYPES(compiler, symtab, node, type, context) { \
  try { \
    mml::type_checker checker(compiler, symtab, this, type, context); \
    (node)->accept(&checker, 0); \
  } \
  catch (const std::string &problem) { \
    std::cerr << (node)->lineno() << ": " << problem << std::endl; \
    return; \
  } \
}

#define ASSERT_SAFE_EXPRESSIONS CHECK_TYPES(_compiler, _symtab, node, functionType(), context())

#endif
