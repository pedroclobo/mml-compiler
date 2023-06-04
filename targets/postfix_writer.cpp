#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include ".auto/all_nodes.h"  // all_nodes.h is automatically generated
#include "mml_parser.tab.h"

//---------------------------------------------------------------------------

void mml::postfix_writer::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void mml::postfix_writer::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void mml::postfix_writer::do_not_node(cdk::not_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(0);
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
void mml::postfix_writer::do_address_of_node(mml::address_of_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
}
void mml::postfix_writer::do_null_node(mml::null_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  if (this->functionBody() || this->functionArgs()) {
    _pf.INT(0);
  } else {
    _pf.SINT(0);
  }
}
void mml::postfix_writer::do_stop_node(mml::stop_node * const node, int lvl) {
  if (node->level() <= (int)_whileEnd.size()) {
    _pf.JMP(mklbl(_whileEnd[_whileEnd.size() - node->level()]));
  }
}
void mml::postfix_writer::do_next_node(mml::next_node * const node, int lvl) {
  if (node->level() <= (int)_whileCond.size()) {
    _pf.JMP(mklbl(_whileCond[_whileCond.size() - node->level()]));
  }
}
void mml::postfix_writer::do_return_node(mml::return_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  auto fun_type = cdk::functional_type::cast(this->functionType());
  auto ret_type = fun_type->output(0);

  if (ret_type->name() != cdk::TYPE_VOID) {
    node->retval()->accept(this, lvl);
    if (ret_type->name() == cdk::TYPE_INT || ret_type->name() == cdk::TYPE_STRING || ret_type->name() == cdk::TYPE_POINTER || ret_type->name() == cdk::TYPE_FUNCTIONAL) {
      _pf.STFVAL32();
    } else if (ret_type->name() == cdk::TYPE_DOUBLE) {
      if (node->retval()->type()->name() == cdk::TYPE_INT) {
        _pf.I2D();
      }
      _pf.STFVAL64();
    } else {
      std::cerr << node->lineno() << ": should not happen: unknown return type" << std::endl;
    }
  }

  _pf.JMP(this->returnLabel());
  this->setReturnSeen(true);
}

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
void mml::postfix_writer::do_stack_alloc_node(mml::stack_alloc_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->argument()->accept(this, lvl);

  auto ref_type = cdk::reference_type::cast(node->type())->referenced();
  if (!ref_type || ref_type->name() != cdk::TYPE_DOUBLE) {
    _pf.INT(4);
  } else {
    _pf.INT(8);
  }

  _pf.MUL();
  _pf.ALLOC();
  _pf.SP();
}
void mml::postfix_writer::do_block_node(mml::block_node * const node, int lvl) {
  node->declarations()->accept(this, lvl);
  node->instructions()->accept(this, lvl);
}

void mml::postfix_writer::do_declaration_node(mml::declaration_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  int offset = 0;
  if (this->functionArgs()) {
    offset = this->offset();
    this->setOffset(this->offset() + node->type()->size());
  } else if (this->functionBody()) {
    this->setOffset(this->offset() - node->type()->size());
    offset = this->offset();
  }

  auto symbol = new_symbol();
  if (symbol) {
    symbol->offset(offset);
    reset_new_symbol();
  }

  if (this->functionBody() || this->functionArgs()) {
    symbol->global(false);
    if (node->initializer()) {
      node->initializer()->accept(this, lvl);
      if (node->is_typed(cdk::TYPE_INT) || node->is_typed(cdk::TYPE_STRING)) {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      } else if (node->is_typed(cdk::TYPE_DOUBLE)) {
        if (node->initializer()->is_typed(cdk::TYPE_INT)) {
          _pf.I2D();
        }
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      }
    }
  } else {
    // FIXME: should add symbol?
    if (node->qualifier() == tFORWARD) {
      return;
    // FIXME: should add symbol?
    } else if (node->qualifier() == tFOREIGN) {
      this->addForeignFunction(node->identifier());
      return;
    }
    symbol->global(true);
    if (!node->initializer()) {
      _pf.BSS();
      _pf.ALIGN();
      if (node->qualifier() == tPUBLIC) {
        _pf.GLOBAL(node->identifier(), _pf.OBJ());
      }
      _pf.LABEL(node->identifier());
      _pf.SALLOC(node->type()->size());
    } else {
      _pf.DATA();
      _pf.ALIGN();
      if (node->qualifier() == tPUBLIC) {
        _pf.GLOBAL(node->identifier(), _pf.OBJ());
      }
      _pf.LABEL(node->identifier());
      node->initializer()->accept(this, lvl);
    }
  }
}

void mml::postfix_writer::do_function_call_node(mml::function_call_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  std::shared_ptr<cdk::basic_type> type;
  if (!node->function()) {
    type = this->functionType();
  } else {
    type = node->function()->type();
  }
  auto func_type = cdk::functional_type::cast(type);

  int argsSize = 0;
  for (int i = node->arguments()->size() - 1; i >= 0; i--) {
    auto arg = dynamic_cast<cdk::expression_node*>(node->arguments()->node(i));
    auto type = func_type->input(i);
    arg->accept(this, lvl);
    if (arg->is_typed(cdk::TYPE_INT) && type->name() == cdk::TYPE_DOUBLE) {
      _pf.I2D();
    }
    argsSize += arg->type()->size();
  }

  if (node->function()) {
    auto rval_node = dynamic_cast<cdk::rvalue_node*>(node->function());
    auto var_node = dynamic_cast<cdk::variable_node*>(rval_node->lvalue());
    auto identifier = var_node->name();

    auto symbol = _symtab.find(identifier);
    if (symbol->qualifier() == tFOREIGN) {
      _pf.CALL(symbol->identifier());
    } else {
      node->function()->accept(this, lvl);
      _pf.BRANCH();
    }
  } else {
    _pf.ADDR(this->textLabel());
    _pf.BRANCH();
  }

  if (argsSize > 0) {
    _pf.TRASH(argsSize);
  }

  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.LDFVAL64();
  } else {
    _pf.LDFVAL32();
  }
}

void mml::postfix_writer::do_function_definition_node(mml::function_definition_node * const node, int lvl) {
  int lbl = ++_lbl;

  this->pushTextLabel(mklbl(lbl));
  this->pushReturnLabel(mklbl(++_lbl));
  this->pushFunctionType(node->type());
  this->pushReturnSeen();

  // calculate function frame size
  frame_size_calculator frame_calc(_compiler, _symtab);
  node->accept(&frame_calc, lvl);

  this->pushFunctionArgs(true);
  this->pushFunctionBody(false);
  this->pushOffset(8);

  _symtab.push();

  node->arguments()->accept(this, lvl);

  // define function in text segment
  _pf.TEXT(mklbl(lbl));
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
  _pf.ENTER(frame_calc.size());

  this->setOffset(0);
  this->setFunctionArgs(false);
  this->setFunctionBody(true);

  if (node->block()) {
    node->block()->accept(this, lvl);
  }

  this->setFunctionBody(false);

  _pf.ALIGN();
  _pf.LABEL(this->returnLabel());
  _pf.LEAVE();
  _pf.RET();

  _symtab.pop();

  this->popFunctionBody();
  this->popFunctionArgs();

  this->popReturnSeen();
  this->popFunctionType();
  this->popReturnLabel();
  this->popTextLabel();

  if (this->functionBody() || this->functionArgs()) {
    if (this->textLabel() == "") {
      _pf.TEXT();
    } else {
      _pf.TEXT(this->textLabel());
    }
    _pf.ADDR(mklbl(lbl));
  } else {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl));
  }
}

void mml::postfix_writer::do_program_node(mml::program_node * const node, int lvl) {
  this->pushOffset(0);

  if (node->declarations()) {
    node->declarations()->accept(this, lvl);
  }

  this->pushFunctionBody(true);

  this->pushReturnLabel(mklbl(++_lbl));
  this->pushFunctionType(cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)));
  this->pushReturnSeen();

  // calculate stack size for main function
  frame_size_calculator frame_calc(_compiler, _symtab);
  node->accept(&frame_calc, lvl);

  // generate the main function (RTS mandates that its name be "_main")
  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL("_main");
  _pf.ENTER(frame_calc.size());

  if (node->block()) {
    node->block()->accept(this, lvl);
  }

  // end the main function
  if (!this->returnSeen()) {
    _pf.INT(0);
    _pf.STFVAL32();
  } else {
    _pf.ALIGN();
    _pf.LABEL(this->returnLabel());
  }
  _pf.LEAVE();
  _pf.RET();

  auto foreigns = this->foreignFunctions();
  for (auto identifier = foreigns.begin(); identifier != foreigns.end(); ++identifier) {
    _pf.EXTERN(*identifier);
  }

  this->popReturnSeen();
  this->popFunctionType();
  this->popReturnLabel();
  this->popOffset();
  this->popFunctionBody();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  if (this->functionBody() || this->functionArgs()) {
    _pf.INT(node->value());
  } else {
    _pf.SINT(node->value());
  }
}

void mml::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  if (this->functionBody() || this->functionArgs()) {
    _pf.DOUBLE(node->value());
  } else {
    _pf.SDOUBLE(node->value());
  }
}

void mml::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value()); // output string characters

  if (this->functionBody() || this->functionArgs()) {
    if (this->textLabel() == "") {
      _pf.TEXT();
    } else {
      _pf.TEXT(this->textLabel());
    }
    _pf.ADDR(mklbl(lbl1));
  } else {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  _pf.NEG(); // 2-complement
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->left()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->left()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    auto ref_type = cdk::reference_type::cast(node->type())->referenced();
    if (ref_type->name() == cdk::TYPE_POINTER || ref_type->name() == cdk::TYPE_INT) {
      _pf.INT(4);
    } else if (ref_type->name() == cdk::TYPE_DOUBLE) {
      _pf.INT(8);
    }
    _pf.MUL();
  }

  node->right()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT)) {
    _pf.I2D();
  } else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    auto ref_type = cdk::reference_type::cast(node->type())->referenced();
    if (ref_type->name() == cdk::TYPE_POINTER || ref_type->name() == cdk::TYPE_INT) {
      _pf.INT(4);
    } else if (ref_type->name() == cdk::TYPE_DOUBLE) {
      _pf.INT(8);
    }
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
  node->right()->accept(this, lvl);
  _pf.SUB();
}
void mml::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MUL();
}
void mml::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.DIV();
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
  node->right()->accept(this, lvl);
  _pf.LT();
}
void mml::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.LE();
}
void mml::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.GE();
}
void mml::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.GT();
}
void mml::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.NE();
}
void mml::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.EQ();
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
    child->accept(this, lvl); // expression to print

    if (child->is_typed(cdk::TYPE_INT)) {
      _pf.CALL("printi");
      _pf.TRASH(4);
      this->addForeignFunction("printi");
    } else if (child->is_typed(cdk::TYPE_STRING)) {
      _pf.CALL("prints");
      _pf.TRASH(4);
      this->addForeignFunction("prints");
    } else if (child->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.CALL("printd");
      _pf.TRASH(8);
      this->addForeignFunction("printd");
    } else {
      std::cerr << "ERROR: CANNOT PRINT EXPRESSION OF UNKNOWN TYPE" << std::endl;
      exit(1);
    }
  }

  if (node->newline()) {
    _pf.CALL("println");
    this->addForeignFunction("println");
  }
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_read_node(mml::read_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.CALL("readi");
  _pf.LDFVAL32();
  _pf.STINT();
  this->addForeignFunction("readi");
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_while_node(mml::while_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _symtab.push();

  int condition, end;
  _whileCond.push_back(condition = ++_lbl);
  _whileEnd.push_back(end = ++_lbl);

  _pf.ALIGN();
  _pf.LABEL(mklbl(condition));
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(end));
  node->block()->accept(this, lvl);
  _pf.JMP(mklbl(condition));
  _pf.ALIGN();
  _pf.LABEL(mklbl(end));

  _whileCond.pop_back();
  _whileEnd.pop_back();

  _symtab.pop();
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_node(mml::if_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void mml::postfix_writer::do_if_else_node(mml::if_else_node * const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1 = lbl2));
}
