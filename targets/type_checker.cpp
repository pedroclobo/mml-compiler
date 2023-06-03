#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>
#include "mml_parser.tab.h"

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

void mml::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}

void mml::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

void mml::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

void mml::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void mml::type_checker::do_null_node(mml::null_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::reference_type::create(4, nullptr));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("not expressions only accept integers");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl);
  if (!node->argument()->is_typed(cdk::TYPE_INT) && !node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    throw std::string("neg expressions only accept integers or doubles");
  }

  if (node->argument()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
  }
}

void mml::type_checker::do_stack_alloc_node(mml::stack_alloc_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl);
  if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("allocation operation only accept integers");
  }

  // TODO: is this right?
  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else if (node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(node->right()->type());
    } else if (node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->type(node->right()->type());
    } else {
      throw std::string("invalid right operand to add to integer");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else {
      throw std::string("invalid right operand to add to double");
    }
  } else if (node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else {
      throw std::string("invalid right operand to add to pointer");
    }
  } else {
    throw std::string("incompatible arguments in add operation");
  }
}

void mml::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else if (node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(node->right()->type());
    } else if (node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->type(node->right()->type());
    } else {
      throw std::string("invalid right operand to add to integer");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else {
      throw std::string("invalid right operand to add to double");
    }
  } else if (node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else if (node->right()->is_typed(cdk::TYPE_POINTER)) {
      auto left_ref = cdk::reference_type::cast(node->left()->type());
      auto right_ref = cdk::reference_type::cast(node->right()->type());

      if (left_ref->referenced() != right_ref->referenced()) {
        throw std::string("pointers must have the same type in sub operations");
      }
      node->type(node->left()->type());
    } else {
      throw std::string("invalid right operand to add to pointer");
    }
  } else {
    throw std::string("incompatible arguments in add operation");
  }
}

void mml::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else if (node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(node->right()->type());
    } else {
      throw std::string("invalid right operand to mul with integer");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else {
      throw std::string("invalid right operand to mul with double");
    }
  } else {
    throw std::string("incompatible arguments in mul operation");
  }
}

void mml::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else if (node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(node->right()->type());
    } else {
      throw std::string("invalid right operand to div integer");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else {
      throw std::string("invalid right operand to div double");
    }
  } else {
    throw std::string("incompatible arguments in div operation");
  }
}

void mml::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (!node->left()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("mod expressions only accept integers");
  }

  node->type(node->left()->type());
}

void mml::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("invalid right operand to lt operation");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (!node->right()->is_typed(cdk::TYPE_DOUBLE) && !node->right()->is_typed(cdk::TYPE_INT)) {
      throw std::string("invalid right operand to lt operation");
    }
  } else if (node->left()->is_typed(cdk::TYPE_STRING) || node->right()->is_typed(cdk::TYPE_STRING)) {
    throw std::string("lt operation doesn't accept strings");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("invalid right operand to le operation");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (!node->right()->is_typed(cdk::TYPE_DOUBLE) && !node->right()->is_typed(cdk::TYPE_INT)) {
      throw std::string("invalid right operand to le operation");
    }
  } else if (node->left()->is_typed(cdk::TYPE_STRING) || node->right()->is_typed(cdk::TYPE_STRING)) {
    throw std::string("le operation doesn't accept strings");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("invalid right operand to ge operation");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (!node->right()->is_typed(cdk::TYPE_DOUBLE) && !node->right()->is_typed(cdk::TYPE_INT)) {
      throw std::string("invalid right operand to ge operation");
    }
  } else if (node->left()->is_typed(cdk::TYPE_STRING) || node->right()->is_typed(cdk::TYPE_STRING)) {
    throw std::string("ge operation doesn't accept strings");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("invalid right operand to gt operation");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (!node->right()->is_typed(cdk::TYPE_DOUBLE) && !node->right()->is_typed(cdk::TYPE_INT)) {
      throw std::string("invalid right operand to gt operation");
    }
  } else if (node->left()->is_typed(cdk::TYPE_STRING) || node->right()->is_typed(cdk::TYPE_STRING)) {
    throw std::string("gt operation doesn't accept strings");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("invalid right operand in ne operation to compare to integer");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (!node->right()->is_typed(cdk::TYPE_DOUBLE) && !node->right()->is_typed(cdk::TYPE_INT)) {
      throw std::string("invalid right operand in ne operation to compare to double");
    }
  } else if (node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (!node->right()->is_typed(cdk::TYPE_POINTER)) {
      throw std::string("invalid right operand in ne operation to compare to pointer");
    }
  } else if (node->left()->is_typed(cdk::TYPE_STRING) || node->right()->is_typed(cdk::TYPE_STRING)) {
    throw std::string("gt operation doesn't accept strings");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("invalid right operand in eq operation to compare to integer");
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (!node->right()->is_typed(cdk::TYPE_DOUBLE) && !node->right()->is_typed(cdk::TYPE_INT)) {
      throw std::string("invalid right operand in eq operation to compare to double");
    }
  } else if (node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (!node->right()->is_typed(cdk::TYPE_POINTER)) {
      throw std::string("invalid right operand in eq operation to compare to pointer");
    }
  } else if (node->left()->is_typed(cdk::TYPE_STRING) || node->right()->is_typed(cdk::TYPE_STRING)) {
    throw std::string("gt operation doesn't accept strings");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("and expressions only accept integers");
  }

  node->right()->accept(this, lvl);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("and expressions only accept integers");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  if (!node->left()->is_typed(cdk::TYPE_INT)) {
    throw std::string("or expressions only accept integers");
  }

  node->right()->accept(this, lvl);
  if (!node->right()->is_typed(cdk::TYPE_INT)) {
    throw std::string("or expressions only accept integers");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;

  auto symbol = _symtab.find(node->name());
  if (!symbol) {
    throw "undeclared variable '" + node->name() + "'";
  }

  node->type(symbol->type());
}

void mml::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());
}

void mml::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl);
  node->rvalue()->accept(this, lvl);

  if (node->lvalue()->is_typed(cdk::TYPE_INT)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(node->lvalue()->type());
    } else if (node->rvalue()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(node->rvalue()->type());
    } else if (node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
      node->type(node->rvalue()->type());
    } else {
      throw std::string("invalid rvalue operand to assign to integer lvalue");
    }
  } else if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_DOUBLE) || node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(node->lvalue()->type());
    } else {
      throw std::string("invalid rvalue operand to assign to double lvalue");
    }
  } else if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(node->lvalue()->type());
    } else if (node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
      auto lvalue_ref = cdk::reference_type::cast(node->lvalue()->type());
      auto rvalue_ref = cdk::reference_type::cast(node->rvalue()->type());

      // TOOO: nullptr
      if (lvalue_ref->referenced() != rvalue_ref->referenced()) {
        throw std::string("assignment of imcompatible pointers");
      }
      node->type(node->lvalue()->type());
    } else {
      throw std::string("invalid rvalue operand to assign to pointer lvalue");
    }
  } else {
    throw std::string("incompatible arguments in assignment operation");
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_evaluation_node(mml::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl);
}

void mml::type_checker::do_print_node(mml::print_node *const node, int lvl) {
  node->arguments()->accept(this, lvl);
  for (size_t i = 0; i < node->arguments()->size(); i++) {
    auto child = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
    if (child->is_typed(cdk::TYPE_POINTER)) {
      throw std::string("pointers cannot be printed");
    }
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_read_node(mml::read_node *const node, int lvl) {
  ASSERT_UNSPEC;

  // read nodes can have type integer or double
  // if a double is needed, the work will be done by the other nodes
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void mml::type_checker::do_while_node(mml::while_node *const node, int lvl) {
  node->condition()->accept(this, lvl);
  node->block()->accept(this, lvl);
}

void mml::type_checker::do_stop_node(mml::stop_node * const node, int lvl) {
  // EMPTY
}

void mml::type_checker::do_next_node(mml::next_node * const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void mml::type_checker::do_if_node(mml::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl);
  node->block()->accept(this, lvl);
}

void mml::type_checker::do_if_else_node(mml::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl);
  node->thenblock()->accept(this, lvl);
  node->elseblock()->accept(this, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_block_node(mml::block_node * const node, int lvl) {
  _symtab.push();

  // TODO: probably needed to calculate frame size
  node->declarations()->accept(this, lvl);
  node->instructions()->accept(this, lvl);

  _symtab.pop();
}

void mml::type_checker::do_declaration_node(mml::declaration_node * const node, int lvl) {
  if (node->initializer()) {
    node->initializer()->accept(this, lvl);

    if (node->is_typed(cdk::TYPE_INT)) {
      if (!node->initializer()->is_typed(cdk::TYPE_INT)) {
        throw std::string("wrong type for initializer: expected integer");
      }
    } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
      if (!node->initializer()->is_typed(cdk::TYPE_INT) && !node->initializer()->is_typed(cdk::TYPE_DOUBLE)) {
        throw std::string("wrong type for initializer: expected integer or double");
      }
    } else if (node->is_typed(cdk::TYPE_STRING)) {
      if (!node->initializer()->is_typed(cdk::TYPE_STRING)) {
        throw std::string("wrong type for initializer: expected string");
      }
    }
    // FIXME: handle other types

    node->type(node->initializer()->type());
  }

  // FIXME: handle other types (function not supported)
  auto symbol = std::make_shared<mml::symbol>(node->type(), node->identifier(), node->qualifier(), false, false);
  if (!_symtab.insert(node->identifier(), symbol)){
    auto old_symbol = _symtab.find(node->identifier());
    if (old_symbol->qualifier() == tFORWARD) {
      _symtab.replace(node->identifier(), symbol);
    } else {
      throw std::string(node->identifier() + " redeclared");
    }
  }

  _parent->set_new_symbol(symbol);
}

void mml::type_checker::do_program_node(mml::program_node *const node, int lvl) {
  node->declarations()->accept(this, lvl);

  _symtab.push();
  node->block()->accept(this, lvl);
  _symtab.pop();
}

//---------------------------------------------------------------------------

void mml::type_checker::do_function_call_node(mml::function_call_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->function()->accept(this, lvl);
  node->arguments()->accept(this, lvl);
  auto func_type = cdk::functional_type::cast(node->function()->type());
  node->type(func_type->output(0));
  // TODO
}

void mml::type_checker::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {
    return;
  }

  std::vector<std::shared_ptr<cdk::basic_type>> input;
  for (size_t i = 0; i < node->arguments()->size(); i++){
    input.push_back(node->argument(i)->type());
  }

  node->type(cdk::functional_type::create(input, node->type()));

  node->arguments()->accept(this, lvl);
  node->block()->accept(this, lvl);
}

void mml::type_checker::do_return_node(mml::return_node * const node, int lvl) {
  node->retval()->accept(this, lvl);
  // TODO
}

//---------------------------------------------------------------------------

void mml::type_checker::do_index_node(mml::index_node * const node, int lvl) {
  // TODO: functions can't be indexed
  ASSERT_UNSPEC;

  node->base()->accept(this, lvl + 2);
  node->index()->accept(this, lvl + 2);

  if (!node->base()->is_typed(cdk::TYPE_POINTER)) {
    throw std::string("only pointers can be indexed");
  }

  if (!node->index()->is_typed(cdk::TYPE_INT)) {
    throw std::string("index must be a integer");
  }

  auto ref_type = std::dynamic_pointer_cast<cdk::reference_type>(node->base()->type());
  node->type(ref_type->referenced());
}

void mml::type_checker::do_sizeof_node(mml::sizeof_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->expression()->accept(this, lvl);
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void mml::type_checker::do_address_of_node(mml::address_of_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}
