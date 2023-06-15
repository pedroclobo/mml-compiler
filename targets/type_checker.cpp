#include <string>
#include "targets/type_checker.h"
#include "targets/context_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>
#include "mml_parser.tab.h"

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

bool matching_references(std::shared_ptr<cdk::basic_type> type1, std::shared_ptr<cdk::basic_type> type2) {
  if (type1->name() != cdk::TYPE_POINTER || type2->name() != cdk::TYPE_POINTER) {
    return false;
  }

  int type1_lvl = 0;
  int type2_lvl = 0;

  while (type1->name() == cdk::TYPE_POINTER) {
    type1 = cdk::reference_type::cast(type1)->referenced();
    type1_lvl++;
  }

  while (type2->name() == cdk::TYPE_POINTER) {
    type2 = cdk::reference_type::cast(type2)->referenced();
    type2_lvl++;
  }

  bool one_is_void = type1->name() == cdk::TYPE_VOID || type2->name() == cdk::TYPE_VOID;
  if (type1_lvl != type2_lvl) {
    return one_is_void;
  } else {
    return type1->name() == type2->name() || one_is_void;
  }
}

bool covariant_functions(std::shared_ptr<cdk::basic_type> type1, std::shared_ptr<cdk::basic_type> type2, bool *covariant) {
  auto func1 = cdk::functional_type::cast(type1);
  auto func2 = cdk::functional_type::cast(type2);

  *covariant = false;

  // check output
  if (func1->output(0)->name() == cdk::TYPE_DOUBLE && func2->output(0)->name() == cdk::TYPE_INT) {
    *covariant = true;
  } else if (func1->output(0)->name() != func2->output(0)->name()) {
    return false;
  }

  // check input
  if (func1->input_length() != func2->input_length()) {
    return false;
  }
  for (size_t i = 0; i < func1->input_length(); i++) {
    if (func1->input(i)->name() == cdk::TYPE_INT) {
      if (func2->input(i)->name() == cdk::TYPE_DOUBLE) {
        *covariant = true;
      } else if (func2->input(i)->name() != cdk::TYPE_INT) {
        return false;
      }
    } else if (func1->input(i)->name() == cdk::TYPE_DOUBLE) {
      if (func2->input(i)->name() != cdk::TYPE_DOUBLE && func2->input(i)->name() != cdk::TYPE_INT) {
        return false;
      }
    } else if (func1->input(i)->name() == cdk::TYPE_STRING) {
      if (func2->input(i)->name() != cdk::TYPE_STRING) {
        return false;
      }
    } else if (func1->input(i)->name() == cdk::TYPE_POINTER) {
      if (func2->input(i)->name() == cdk::TYPE_POINTER) {
        return matching_references(func1->input(i), func2->input(i));
      }
    } else if (func1->input(i)->name() == cdk::TYPE_FUNCTIONAL) {
      if (func2->input(i)->name() == cdk::TYPE_FUNCTIONAL) {
        return covariant_functions(func1->input(i), func2->input(i), covariant);
      }
    }
  }

  return true;
}


mml::function_definition_node * covariant_function(mml::function_definition_node *function, std::shared_ptr<cdk::basic_type> type) {
  auto func_type = cdk::functional_type::cast(type);

  // create arguments
  auto arguments = new cdk::sequence_node(0);
  for (size_t i = 0; i < function->arguments()->size(); i++) {
    auto old_decl = dynamic_cast<mml::declaration_node *>(function->arguments()->node(i));
    auto decl = new mml::declaration_node(0, tPRIVATE, func_type->input(i), old_decl->identifier(), nullptr);
    arguments = new cdk::sequence_node(0, decl, arguments);
  }

  mml::function_definition_node *cov_func = new mml::function_definition_node(0, arguments, func_type->output(0), function->block());

  return cov_func;
}

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

  if (node->argument()->is_typed(cdk::TYPE_INT)) {
    node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (node->argument()->is_typed(cdk::TYPE_DOUBLE)) {
    node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));

  } else {
    throw std::string("neg expressions only accept integers or doubles");
  }
}

void mml::type_checker::do_stack_alloc_node(mml::stack_alloc_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl);

  if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("allocation operation only accept integers");
  }

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
    if (cdk::reference_type::cast(node->left()->type())->referenced()->name() == cdk::TYPE_FUNCTIONAL) {
      throw std::string("function pointers don't support arithmetic");
    }
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
    if (cdk::reference_type::cast(node->left()->type())->referenced()->name() == cdk::TYPE_FUNCTIONAL) {
      throw std::string("function pointers don't support arithmetic");
    }

    if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else if (node->right()->is_typed(cdk::TYPE_POINTER)) {
      auto l_ref = cdk::reference_type::cast(node->left()->type());
      auto r_ref = cdk::reference_type::cast(node->right()->type());

      // left or right is null
      if (!l_ref->referenced() || !r_ref->referenced()) {
        // the type doesn't matter as the results is undefined
        node->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_UNSPEC)));
        return;
      } else if (!matching_references(node->left()->type(), node->right()->type())) {
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
    if (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }

  } else {
    throw std::string("invalid arguments to lt operation");
  }
}

void mml::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }

  } else {
    throw std::string("invalid arguments to le operation");
  }
}

void mml::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }

  } else {
    throw std::string("invalid arguments to ge operation");
  }
}

void mml::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }

  } else {
    throw std::string("invalid arguments to gt operation");
  }
}

void mml::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else if (node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else {
    throw std::string("invalid arguments to ne operation");
  }
}

void mml::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    if (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else if (node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->right()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else if (node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }
  } else {
    throw std::string("invalid arguments to eq operation");
  }
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

    // HACK: needed to support pointer subtraction
    } else if (node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
      node->type(node->lvalue()->type());

    } else {
      throw std::string("invalid rvalue operand to assign to integer lvalue");
    }

  } else if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_DOUBLE) || node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(node->lvalue()->type());
    } else {
      throw std::string("invalid rvalue operand to assign to double lvalue");
    }

  } else if (node->lvalue()->is_typed(cdk::TYPE_STRING)) {
    if (node ->rvalue()->is_typed(cdk::TYPE_STRING)) {
      node->type(node->lvalue()->type());
    } else {
      throw std::string("invalid rvalue operand to assign to string lvalue");
    }

  } else if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    if (node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
      auto rvalue_ref = cdk::reference_type::cast(node->rvalue()->type())->referenced();

      // right is null
      if (!rvalue_ref) {
        // EMPTY

      // <type> f = [<int>];
      } else if (rvalue_ref->name() == cdk::TYPE_UNSPEC) {
        node->rvalue()->type(node->lvalue()->type());

      } else if (!matching_references(node->lvalue()->type(), node->rvalue()->type())) {
        throw std::string("assignment of incompatible pointers");
      }

      node->type(node->lvalue()->type());

    } else {
      throw std::string("invalid rvalue operand to assign to pointer lvalue");
    }

  } else if (node->lvalue()->is_typed(cdk::TYPE_FUNCTIONAL)) {
    if (node->rvalue()->is_typed(cdk::TYPE_FUNCTIONAL)) {
      bool covariant;
      if (!covariant_functions(node->lvalue()->type(), node->rvalue()->type(), &covariant)) {
        throw std::string("assignment of incompatible functions");
      }

      node->type(node->lvalue()->type());
    } else {
      throw std::string("invalid rvalue operand to assign to function lvalue");
    }
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
    } else if (child->is_typed(cdk::TYPE_FUNCTIONAL)) {
      throw std::string("functions cannot be printed");
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

  if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string("expected integer condition");
  }
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

  if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string("expected integer condition");
  }
}

void mml::type_checker::do_if_else_node(mml::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl);

  if (!node->condition()->is_typed(cdk::TYPE_INT)) {
    throw std::string("expected integer condition");
  }
}

//---------------------------------------------------------------------------

void mml::type_checker::do_block_node(mml::block_node * const node, int lvl) {
  // EMPTY
}

void mml::type_checker::do_declaration_node(mml::declaration_node * const node, int lvl) {
  if (node->initializer()) {
    node->initializer()->accept(this, lvl);

    if (node->initializer()->is_typed(cdk::TYPE_VOID)) {
      throw std::string("wrong type for initializer: void");
    }

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
    } else if (node->is_typed(cdk::TYPE_POINTER)) {
      if (!node->initializer()->is_typed(cdk::TYPE_POINTER)) {
        throw std::string("wrong type for initializer: expected pointer");
      }

      auto init_ref_type = cdk::reference_type::cast(node->initializer()->type())->referenced();

      // [type] id = null;
      if (!init_ref_type) {
        node->initializer()->type(node->type());

      // [type] id = [int];
      } else if (init_ref_type->name() == cdk::TYPE_UNSPEC) {
        node->initializer()->type(node->type());

      } else {
        if (!matching_references(node->type(), node->initializer()->type())) {
          throw std::string("wrong type for initializer: incompatible pointer types");
        }
      }
    } else if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {
      if (!node->initializer()->is_typed(cdk::TYPE_FUNCTIONAL)) {
        throw std::string("wrong type for initializer: expected function");
      }
      bool covariant;
      if (!covariant_functions(node->type(), node->initializer()->type(), &covariant)) {
        // TODO: create covariant function
        throw std::string("wrong type for initializer: incompatible function");
      }
      if (covariant) {

        // check for variable_node
        auto rval_node = dynamic_cast<cdk::rvalue_node*>(node->initializer());
        if (rval_node) {
          auto var_node = dynamic_cast<cdk::variable_node*>(rval_node->lvalue());
          auto identifier = var_node->name();
          auto symbol = _symtab.find(identifier);
          auto function = (mml::function_definition_node*)symbol->value();
          node->initializer(covariant_function(function, node->type()));
        } else {
          auto function = dynamic_cast<mml::function_definition_node*>(node->initializer());
          node->initializer(covariant_function(function, node->type()));
        }
      }
    // auto
    } else if (node->is_typed(cdk::TYPE_UNSPEC)) {
      node->type(node->initializer()->type());
    }
  }

  auto symbol = std::make_shared<mml::symbol>(node->type(), node->identifier(), node->qualifier());
  if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {
    symbol->value(node->initializer());
  }
  if (node->qualifier() == tFORWARD) {
    symbol->global(true);
  }
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
  mml::context_checker cc(_compiler);
  node->accept(&cc, lvl);
}

//---------------------------------------------------------------------------

void mml::type_checker::do_function_call_node(mml::function_call_node * const node, int lvl) {
  ASSERT_UNSPEC;

  std::shared_ptr<cdk::basic_type> type;
  if (!node->function()) {
    type = _type;
  } else {
    node->function()->accept(this, lvl);
    type = node->function()->type();
  }

  auto func_type = cdk::functional_type::cast(type);
  auto arguments = new cdk::sequence_node(node->lineno());

  for (size_t i = 0; i < node->arguments()->size(); i++) {
    node->argument(i)->accept(this, lvl);
    cdk::expression_node *argument;

    if (node->argument(i)->is_typed(cdk::TYPE_FUNCTIONAL)) {

      if (func_type->input(i)->name() != cdk::TYPE_FUNCTIONAL) {
        throw std::string("wrong type for initializer: expected function"); // FIXME
      }
      bool covariant;
      if (!covariant_functions(func_type->input(i), node->argument(i)->type(), &covariant)) {
        // TODO: create covariant function
        throw std::string("wrong type for initializer: incompatible function"); // FIXME
      }
      if (covariant) {

        // check for variable_node
        auto rval_node = dynamic_cast<cdk::rvalue_node*>(node->argument(i));
        if (rval_node) {
          auto var_node = dynamic_cast<cdk::variable_node*>(rval_node->lvalue());
          auto identifier = var_node->name();
          auto symbol = _symtab.find(identifier);
          auto function = (mml::function_definition_node*)symbol->value();

          argument = covariant_function(function, func_type->input(i));
        } else {
          auto function = dynamic_cast<mml::function_definition_node*>(node->argument(i));
          argument = covariant_function(function, func_type->input(i));
        }
      } else {
        argument = node->argument(i);
      }

    } else {
      argument = node->argument(i);
    }
    arguments = new cdk::sequence_node(node->lineno(), argument, arguments);
  }

  node->arguments(arguments);
  node->type(func_type->output(0));
  // TODO
}

void mml::type_checker::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  mml::context_checker cc(_compiler);
  node->accept(&cc, lvl);
}

void mml::type_checker::do_return_node(mml::return_node * const node, int lvl) {
  if (node->retval()) {
    node->retval()->accept(this, lvl);
  }
  // TODO
}

//---------------------------------------------------------------------------

void mml::type_checker::do_index_node(mml::index_node * const node, int lvl) {
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
