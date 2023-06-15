#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include "targets/context.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated
#include "mml_parser.tab.h"

//---------------------------------------------------------------------------

static bool covariant_functions(std::shared_ptr<cdk::basic_type> type1, std::shared_ptr<cdk::basic_type> type2) {
  if (type1->name() != cdk::TYPE_FUNCTIONAL || type2->name() != cdk::TYPE_FUNCTIONAL) {
    return false;
  }

  auto func1 = cdk::functional_type::cast(type1);
  auto func2 = cdk::functional_type::cast(type2);

  bool covariant = false;

  if (func1->output(0)->name() == cdk::TYPE_DOUBLE && func2->output(0)->name() == cdk::TYPE_INT) {
    covariant = true;
  }

  for (size_t i = 0; i < func1->input_length(); i++) {
    if (func1->input(i)->name() == cdk::TYPE_INT) {
      if (func2->input(i)->name() == cdk::TYPE_DOUBLE) {
        covariant = true;
      }
    } else if (func1->input(i)->name() == cdk::TYPE_FUNCTIONAL) {
      if (func2->input(i)->name() == cdk::TYPE_FUNCTIONAL) {
        covariant = covariant_functions(func1->input(i), func2->input(i));
      }
    }
  }

  return covariant;
}


static mml::function_definition_node * covariant_function(mml::function_definition_node *function, std::shared_ptr<cdk::basic_type> type) {
  auto funcType = cdk::functional_type::cast(type);

  // change argument types
  auto arguments = new cdk::sequence_node(0);
  for (size_t i = 0; i < function->arguments()->size(); i++) {
    auto old_decl = dynamic_cast<mml::declaration_node *>(function->arguments()->node(i));
    auto decl = new mml::declaration_node(0, tPRIVATE, funcType->input(i), old_decl->identifier(), nullptr);
    arguments = new cdk::sequence_node(0, decl, arguments);
  }

  mml::function_definition_node *cov_func = new mml::function_definition_node(0, arguments, funcType->output(0), function->block());

  return cov_func;
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}

void mml::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}

void mml::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (this->context() == mml::CONTEXT_GLOBAL) {
    _pf.SINT(node->value());
  } else {
    _pf.INT(node->value());
  }
}

void mml::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl;

  _pf.RODATA();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl = ++_lbl));
  _pf.SSTRING(node->value());

  if (this->context() == mml::CONTEXT_GLOBAL) {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl));
  } else {
    _pf.TEXT(this->textLabel());
    _pf.ADDR(mklbl(lbl));
  }
}

void mml::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (this->context() == mml::CONTEXT_GLOBAL) {
    _pf.SDOUBLE(node->value());
  } else {
    int lbl;
    _pf.RODATA();
    _pf.ALIGN();
    _pf.LABEL(mklbl(lbl = ++_lbl));
    _pf.SDOUBLE(node->value());

    _pf.TEXT(this->textLabel());
    _pf.ADDR(mklbl(lbl));
    _pf.LDDOUBLE();
  }
}

void mml::postfix_writer::do_null_node(mml::null_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (this->context() == mml::CONTEXT_GLOBAL) {
    _pf.SINT(0);
  } else {
    _pf.INT(0);
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(0);
  _pf.EQ();
}

void mml::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.NEG();
}

void mml::postfix_writer::do_stack_alloc_node(mml::stack_alloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->argument()->accept(this, lvl);

  auto ref_type = cdk::reference_type::cast(node->type())->referenced();
  if (ref_type->name() == cdk::TYPE_DOUBLE) {
    _pf.INT(8);
  } else {
    _pf.INT(4);
  }

  _pf.MUL();
  _pf.ALLOC();
  _pf.SP();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    auto ref_type = cdk::reference_type::cast(node->type())->referenced();
    _pf.INT(ref_type->size());
    _pf.MUL();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    auto ref_type = cdk::reference_type::cast(node->type())->referenced();
    _pf.INT(ref_type->size());
    _pf.MUL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DADD();
  } else {
    _pf.ADD();
  }
}

void mml::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    auto ref_type = cdk::reference_type::cast(node->type())->referenced();
    _pf.INT(ref_type->size());
    _pf.MUL();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    auto ref_type = cdk::reference_type::cast(node->type())->referenced();
    _pf.INT(ref_type->size());
    _pf.MUL();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DSUB();
  } else if (node->is_typed(cdk::TYPE_POINTER)) {
    auto ref_type = cdk::reference_type::cast(node->type())->referenced();
    _pf.SUB();
    _pf.INT(ref_type->size());
    _pf.DIV();
  } else {
    _pf.SUB();
  }
}

void mml::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DMUL();
  } else {
    _pf.MUL();
  }
}

void mml::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DDIV();
  } else {
    _pf.DIV();
  }
}

void mml::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}

void mml::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  _pf.LT();
}

void mml::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  _pf.LE();
}

void mml::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  _pf.GE();
}

void mml::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  _pf.GT();
}

void mml::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  _pf.NE();
}

void mml::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  }

  _pf.EQ();
}

void mml::postfix_writer::do_and_node(cdk::and_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int lbl = ++_lbl;

  node->left()->accept(this, lvl);
  _pf.DUP32();
  _pf.JZ(mklbl(lbl));

  node->right()->accept(this, lvl);
  _pf.AND();

  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

void mml::postfix_writer::do_or_node(cdk::or_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int lbl = ++_lbl;

  node->left()->accept(this, lvl);
  _pf.DUP32();
  _pf.JNZ(mklbl(lbl));

  node->right()->accept(this, lvl);
  _pf.OR();

  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_variable_node(cdk::variable_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto symbol = _symtab.find(node->name());
  if (symbol->global()) {
    _pf.ADDR(symbol->identifier());
  } else {
    _pf.LOCAL(symbol->offset());
  }
}

void mml::postfix_writer::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDDOUBLE();
  } else {
    _pf.LDINT();
  }
}

void mml::postfix_writer::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  // create a new function definition node when the types are not the same but are covariant
  // this should be done by the type checker
  // however, the assignment node's rvalue can't be changed, so it has do be done here
  if (covariant_functions(node->lvalue()->type(), node->rvalue()->type())) {

    // rvalue is a variable
    auto rval_node = dynamic_cast<cdk::rvalue_node*>(node->rvalue());
    if (rval_node) {
      auto var_node = dynamic_cast<cdk::variable_node*>(rval_node->lvalue());
      auto identifier = var_node->name();
      auto symbol = _symtab.find(identifier);
      auto function = (mml::function_definition_node*)symbol->value();
      covariant_function(function, node->lvalue()->type())->accept(this, lvl);

    // rvalue is a function literal
    } else {
      auto function = dynamic_cast<mml::function_definition_node*>(node->rvalue());
      covariant_function(function, node->lvalue()->type())->accept(this, lvl);
    }

    _pf.DUP32();
    node->lvalue()->accept(this, lvl);
    _pf.STINT();

    return;
  }

  node->rvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      _pf.I2D();
    }
    _pf.DUP64();
  } else {
    _pf.DUP32();
  }

  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.STDOUBLE();
  } else {
    _pf.STINT();
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_evaluation_node(mml::evaluation_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->argument()->accept(this, lvl);
  if (node->argument()->type()->size() != 0) {
    _pf.TRASH(node->argument()->type()->size());
  }
}

void mml::postfix_writer::do_print_node(mml::print_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  for (size_t i = 0; i < node->arguments()->size(); i++) {
    auto child = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
    child->accept(this, lvl);

    if (child->is_typed(cdk::TYPE_INT)) {
      this->addForeignFunction("printi");
      _pf.CALL("printi");
      _pf.TRASH(4);
    } else if (child->is_typed(cdk::TYPE_STRING)) {
      this->addForeignFunction("prints");
      _pf.CALL("prints");
      _pf.TRASH(4);
    } else if (child->is_typed(cdk::TYPE_DOUBLE)) {
      this->addForeignFunction("printd");
      _pf.CALL("printd");
      _pf.TRASH(8);
    } else {
      std::cerr << "ERROR: CANNOT PRINT EXPRESSION OF UNKNOWN TYPE" << std::endl;
      exit(1);
    }
  }

  if (node->newline()) {
    this->addForeignFunction("println");
    _pf.CALL("println");
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_read_node(mml::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (node->is_typed(cdk::TYPE_INT)) {
    this->addForeignFunction("readi");
    _pf.CALL("readi");
    _pf.LDFVAL32();
  } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
    this->addForeignFunction("readd");
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else {
    std::cerr << "ERROR: CANNOT READ EXPRESSION OF UNKNOWN TYPE" << std::endl;
    exit(1);
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_while_node(mml::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int condition, end;
  this->pushCondLabel(condition = ++_lbl);
  this->pushEndLabel(end = ++_lbl);

  _pf.ALIGN();
  _pf.LABEL(mklbl(condition));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(end));
  node->block()->accept(this, lvl);
  _pf.JMP(mklbl(condition));
  _pf.ALIGN();
  _pf.LABEL(mklbl(end));

  this->popEndLabel();
  this->popCondLabel();
}

void mml::postfix_writer::do_stop_node(mml::stop_node * const node, int lvl) {
  // the type checker guarantees that the stop level is valid
  _pf.JMP(mklbl(this->endLabel(node->level())));
}

void mml::postfix_writer::do_next_node(mml::next_node * const node, int lvl) {
  // the type checker guarantees that the next level is valid
  _pf.JMP(mklbl(this->condLabel(node->level())));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_node(mml::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int end;

  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(end = ++_lbl));
  node->block()->accept(this, lvl);
  _pf.ALIGN();
  _pf.LABEL(mklbl(end));
}

void mml::postfix_writer::do_if_else_node(mml::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int else_lbl, end;

  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(else_lbl = ++_lbl));
  node->thenblock()->accept(this, lvl);
  _pf.JMP(mklbl(end = ++_lbl));
  _pf.ALIGN();
  _pf.LABEL(mklbl(else_lbl));
  node->elseblock()->accept(this, lvl);
  _pf.ALIGN();
  _pf.LABEL(mklbl(end));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_block_node(mml::block_node * const node, int lvl) {
  _symtab.push();
  node->declarations()->accept(this, lvl);
  node->instructions()->accept(this, lvl);
  _symtab.pop();
}

void mml::postfix_writer::do_declaration_node(mml::declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int offset = 0;
  if (this->context() == mml::CONTEXT_FUNCTION_ARGS) {
    offset = this->offset();
    this->setOffset(this->offset() + node->type()->size());
  } else if (this->context() == mml::CONTEXT_FUNCTION_BODY || this->context() == mml::CONTEXT_MAIN_BODY) {
    this->setOffset(this->offset() - node->type()->size());
    offset = this->offset();
  }

  auto symbol = new_symbol();
  if (symbol) {
    symbol->offset(offset);
    reset_new_symbol();
  }

  if (node->qualifier() == tFORWARD) {
    return;
  } else if (node->qualifier() == tFOREIGN) {
    this->addForeignFunction(node->identifier());
    return;
  }

  if (this->context() == mml::CONTEXT_GLOBAL) {
    symbol->global(true);

    if (!node->initializer()) {
      _pf.BSS();
      _pf.ALIGN();
      if (node->qualifier() == tPUBLIC) {
        if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {
          _pf.GLOBAL(node->identifier(), _pf.FUNC());
        } else {
          _pf.GLOBAL(node->identifier(), _pf.OBJ());
        }
      }
      _pf.LABEL(node->identifier());
      _pf.SALLOC(node->type()->size());
    } else {
      _pf.DATA();
      _pf.ALIGN();
      if (node->qualifier() == tPUBLIC) {
        if (node->is_typed(cdk::TYPE_FUNCTIONAL)) {
          _pf.GLOBAL(node->identifier(), _pf.FUNC());
        } else {
          _pf.GLOBAL(node->identifier(), _pf.OBJ());
        }
      }
      _pf.LABEL(node->identifier());

      // avoid initializer being an integer_node in `double d = 1;`
      if (node->is_typed(cdk::TYPE_DOUBLE) && node->initializer()->is_typed(cdk::TYPE_INT)) {
        auto init_node = dynamic_cast<cdk::integer_node*>(node->initializer());
        _pf.SDOUBLE(init_node->value());
      } else {
        node->initializer()->accept(this, lvl);
      }
    }
  } else {
    symbol->global(false);

    if (node->initializer()) {
      node->initializer()->accept(this, lvl);
      if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT)) {
          _pf.I2D();
        }
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      } else {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      }
    }
  }
}

void mml::postfix_writer::do_program_node(mml::program_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  this->setOffset(0);

  this->pushContext(mml::CONTEXT_GLOBAL);
  if (node->declarations()) {
    node->declarations()->accept(this, lvl);
  }
  this->popContext();

  this->pushTextLabel("_main");
  this->pushReturnLabel(++_lbl);
  this->pushFunctionType(cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)));
  this->pushReturnSeen();

  // calculate stack size for main function
  frame_size_calculator frame_calc(_compiler, _symtab);
  node->accept(&frame_calc, lvl);

  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT("_main");
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");
  _pf.ENTER(frame_calc.size());

  this->pushContext(mml::CONTEXT_MAIN_BODY);
  if (node->block()) {
    node->block()->accept(this, lvl);
  }
  this->popContext();

  // end the main function
  if (!this->returnSeen()) {
    _pf.INT(0);
    _pf.STFVAL32();
  } else {
    _pf.ALIGN();
    _pf.LABEL(mklbl(this->returnLabel()));
  }
  _pf.LEAVE();
  _pf.RET();

  for (auto identifier: this->foreignFunctions()) {
    _pf.EXTERN(identifier);
  }

  this->popReturnSeen();
  this->popFunctionType();
  this->popReturnLabel();
  this->popTextLabel();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_function_call_node(mml::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  std::shared_ptr<cdk::functional_type> funcType;

  // recursive call
  if (!node->function()) {
    funcType = cdk::functional_type::cast(this->functionType());
  } else {
    funcType = cdk::functional_type::cast(node->function()->type());
  }

  // arguments go in the stack in reverse
  int args_size = 0;
  for (int i = node->arguments()->size() - 1; i >= 0; i--) {
    auto arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
    arg->accept(this, lvl);
    if (funcType->input(i)->name() == cdk::TYPE_DOUBLE && arg->is_typed(cdk::TYPE_INT)) {
      _pf.I2D();
    }
    args_size += funcType->input(i)->size();
  }

  if (node->function()) {
    // function is a variable
    auto rval_node = dynamic_cast<cdk::rvalue_node*>(node->function());
    if (rval_node) {
      auto var_node = dynamic_cast<cdk::variable_node*>(rval_node->lvalue());
      auto identifier = var_node->name();

      auto symbol = _symtab.find(identifier);

      if (symbol->qualifier() == tFOREIGN) {
        _pf.CALL(symbol->identifier());
      } else {
        node->function()->accept(this, lvl);
        _pf.BRANCH();
      }

    // function is a lambda
    } else {
      node->function()->accept(this, lvl);
      _pf.BRANCH();
    }

  // recursive call
  } else {
    _pf.ADDR(this->textLabel());
    _pf.BRANCH();
  }

  if (args_size > 0) {
    _pf.TRASH(args_size);
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else {
    _pf.LDFVAL32();
  }
}

void mml::postfix_writer::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int lbl = ++_lbl;

  this->pushTextLabel(mklbl(lbl));
  this->pushReturnLabel(++_lbl);
  this->pushFunctionType(node->type());
  this->pushReturnSeen();

  // calculate function frame size
  frame_size_calculator frame_calc(_compiler, _symtab);
  node->accept(&frame_calc, lvl);

  _symtab.push();

  this->setOffset(8);
  this->pushContext(mml::CONTEXT_FUNCTION_ARGS);

  node->arguments()->accept(this, lvl);

  // define function in text segment
  _pf.TEXT(mklbl(lbl));
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
  _pf.ENTER(frame_calc.size());

  this->popContext();
  this->setOffset(0);

  this->pushContext(mml::CONTEXT_FUNCTION_BODY);
  if (node->block()) {
    node->block()->accept(this, lvl);
  }
  this->popContext();

  _pf.ALIGN();
  _pf.LABEL(mklbl(this->returnLabel()));
  _pf.LEAVE();
  _pf.RET();

  _symtab.pop();

  this->popReturnSeen();
  this->popFunctionType();
  this->popReturnLabel();
  this->popTextLabel();

  if (this->context() == mml::CONTEXT_GLOBAL) {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl));
  } else {
    _pf.TEXT(this->textLabel());
    _pf.ADDR(mklbl(lbl));
  }
}

void mml::postfix_writer::do_return_node(mml::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto fun_type = cdk::functional_type::cast(this->functionType());
  auto ret_type = fun_type->output(0);

  if (ret_type->name() != cdk::TYPE_VOID) {
    node->retval()->accept(this, lvl);

    if (ret_type->name() == cdk::TYPE_DOUBLE) {
      if (node->retval()->type()->name() == cdk::TYPE_INT) {
        _pf.I2D();
      }
      _pf.STFVAL64();
    } else {
      _pf.STFVAL32();
    }
  }

  _pf.JMP(mklbl(this->returnLabel()));
  this->setReturnSeen(true);
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_index_node(mml::index_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.INT(8);
  } else {
    _pf.INT(4);
  }

  _pf.MUL();
  _pf.ADD();
}

void mml::postfix_writer::do_sizeof_node(mml::sizeof_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->expression()->type()->size());
}

void mml::postfix_writer::do_address_of_node(mml::address_of_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
}
