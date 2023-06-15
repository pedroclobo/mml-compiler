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
  this->setReturnSeen(false);
  this->setStopOrNextSeen(false);

  for (size_t i = 0; i < node->size(); i++) {
    if (this->returnSeen()) {
      throw std::string("return must be the last instruction in a block");
    } else if (this->stopOrNextSeen()) {
      throw std::string("stop/next must be the last instruction in a block");
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
  this->setCycleDepth(this->cycleDepth() + 1);

  this->setReturnSeen(false);
  this->setStopOrNextSeen(false);

  node->block()->accept(this, lvl);

  // this->setStopOrNextSeen(false);
  this->setReturnSeen(false);

  this->setCycleDepth(this->cycleDepth() - 1);
}

void mml::context_checker::do_stop_node(mml::stop_node * const node, int lvl) {
  if (node->level() > this->cycleDepth()) {
    throw std::string("invalid stop");
  }

  this->setStopOrNextSeen(true);
}

void mml::context_checker::do_next_node(mml::next_node * const node, int lvl) {
  if (node->level() > this->cycleDepth()) {
    throw std::string("invalid next");
  }

  this->setStopOrNextSeen(true);
}

//---------------------------------------------------------------------------

void mml::context_checker::do_if_node(mml::if_node * const node, int lvl) {
  node->block()->accept(this, lvl);

  this->setReturnSeen(false);
  this->setStopOrNextSeen(false);
}

void mml::context_checker::do_if_else_node(mml::if_else_node * const node, int lvl) {
  this->setReturnSeen(false);
  this->setStopOrNextSeen(false);

  node->thenblock()->accept(this, lvl);

  bool then_return_seen = this->returnSeen();
  bool then_stop_or_next_seen = this->stopOrNextSeen();

  // check else block
  this->setReturnSeen(false);
  this->setStopOrNextSeen(false);

  node->elseblock()->accept(this, lvl);

  // both blocks must agree
  this->setReturnSeen(this->returnSeen() && then_return_seen);
  this->setStopOrNextSeen(this->stopOrNextSeen() && then_stop_or_next_seen);
}

//---------------------------------------------------------------------------

void mml::context_checker::do_block_node(mml::block_node * const node, int lvl) {
  this->setReturnSeen(false);
  this->setStopOrNextSeen(false);

  node->instructions()->accept(this, lvl);

  this->setStopOrNextSeen(false);
  this->setReturnSeen(false);
}

void mml::context_checker::do_declaration_node(mml::declaration_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_program_node(mml::program_node * const node, int lvl) {
  this->setReturnSeen(false);
  node->block()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void mml::context_checker::do_function_call_node(mml::function_call_node * const node, int lvl) {
  // EMPTY
}

void mml::context_checker::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  this->setReturnSeen(false);
  node->block()->accept(this, lvl);
}

void mml::context_checker::do_return_node(mml::return_node * const node, int lvl) {
  this->setReturnSeen(true);
  this->setStopOrNextSeen(true);
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
