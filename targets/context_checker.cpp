#include <string>
#include "targets/context_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include "mml_parser.tab.h"

//---------------------------------------------------------------------------

void mml::context_checker::do_null_node(mml::null_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::context_checker::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  _return_seen = false;
  _stop_or_next_seen = false;

  for (size_t i = 0; i < node->size(); i++) {
    if (_return_seen) {
      throw std::string("return must be the last instruction in a block");
    } else if (_stop_or_next_seen) {
      throw std::string("break/continue must be the last instruction in a block");
    }

    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void mml::context_checker::do_integer_node(cdk::integer_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_string_node(cdk::string_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_double_node(cdk::double_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::context_checker::do_not_node(cdk::not_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_neg_node(cdk::neg_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_stack_alloc_node(mml::stack_alloc_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::context_checker::do_add_node(cdk::add_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_sub_node(cdk::sub_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_mul_node(cdk::mul_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_div_node(cdk::div_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_mod_node(cdk::mod_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_lt_node(cdk::lt_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_le_node(cdk::le_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_ge_node(cdk::ge_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_gt_node(cdk::gt_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_ne_node(cdk::ne_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_eq_node(cdk::eq_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_and_node(cdk::and_node * const node, int lvl) {
  // EMPTY
}
void mml::context_checker::do_or_node(cdk::or_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::context_checker::do_variable_node(cdk::variable_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::context_checker::do_evaluation_node(mml::evaluation_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_print_node(mml::print_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::context_checker::do_read_node(mml::read_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::context_checker::do_while_node(mml::while_node * const node, int lvl) {
  _cycle_depth++;

  _return_seen = false;
  _stop_or_next_seen = false;
  node->block()->accept(this, lvl);
  _return_seen = false;

  _cycle_depth--;
}

void mml::context_checker::do_stop_node(mml::stop_node * const node, int lvl) {
  if (node->level() > _cycle_depth) {
    throw std::string("invalid stop");
  }
  _stop_or_next_seen = true;
}

void mml::context_checker::do_next_node(mml::next_node * const node, int lvl) {
  if (node->level() > _cycle_depth) {
    throw std::string("invalid next");
  }
  _stop_or_next_seen = true;
}

//---------------------------------------------------------------------------

void mml::context_checker::do_if_node(mml::if_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_if_else_node(mml::if_else_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::context_checker::do_block_node(mml::block_node * const node, int lvl) {
  _return_seen = false;
  _stop_or_next_seen = false;
  // TODO: accept declarations?
  node->instructions()->accept(this, lvl);
}

void mml::context_checker::do_declaration_node(mml::declaration_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_program_node(mml::program_node * const node, int lvl) {
  _return_seen = false;
  node->block()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void mml::context_checker::do_function_call_node(mml::function_call_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  _return_seen = false;
  node->block()->accept(this, lvl);

  if (!_return_seen && !node->is_typed(cdk::TYPE_VOID)) {
    std::cerr << node->lineno() << ": WARNING: function may not always return" << std::endl;
  }
}

void mml::context_checker::do_return_node(mml::return_node * const node, int lvl) {
  _return_seen = true;
  _stop_or_next_seen = true;
}

//---------------------------------------------------------------------------

void mml::context_checker::do_index_node(mml::index_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_sizeof_node(mml::sizeof_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_address_of_node(mml::address_of_node * const node, int lvl) {
  // EMPTY
}
