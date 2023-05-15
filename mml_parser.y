%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                   i;  /* integer value */
  double                d;  /* double value */
  std::string          *s;  /* symbol name or string literal */

  cdk::basic_node      *node;       /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;

  mml::block_node      *block;
};

%token <i> tINTEGER tQUALIFIER
%token <d> tDOUBLE
%token <s> tIDENTIFIER tSTRING
%token tTYPE_INT tTYPE_DOUBLE tTYPE_STRING tTYPE_VOID tTYPE_AUTO
%token tPUBLIC tFORWARD tFOREIGN
%token tIF tELIF tELSE tWHILE tSTOP tNEXT tRETURN
%token tPRINT tPRINTLN tREAD tNULL tSIZEOF
%token tBEGIN tEND
%token tAND tOR tNE tLE tGE

%right '='
%left tOR
%left tAND
%nonassoc '~'
%left tNE tEQ
%left '<' tLE tGE '>'
%left '+' '-'
%left '*' '/' '%'
%nonassoc tUNARY
%nonassoc '(' ')' '[' ']'

%nonassoc tIF
%nonassoc tELSE tELIF

%type <node> file instruction if_instruction declaration variable
%type <sequence> instructions declarations expressions variables
%type <expression> expression initializer opt_initializer
%type <block> block
%type <lvalue> lvalue
%type <type> data_type opt_data_type function_type
%type <s> string
%type <i> qualifier opt_qualifier

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file           : declarations                                        { compiler->ast(new mml::program_node(LINE, $1, nullptr));         }
               | tBEGIN block tEND                                   { compiler->ast(new mml::program_node(LINE, nullptr, $2));         }
               | declarations tBEGIN block tEND                      { compiler->ast(new mml::program_node(LINE, $1, $3));              }
               ;

block          : declarations instructions                           { $$ = new mml::block_node(LINE, $1, $2);                          }
               | declarations                                        { $$ = new mml::block_node(LINE, $1, nullptr);                     }
               | instructions                                        { $$ = new mml::block_node(LINE, nullptr, $1);                     }
               | /* empty */                                         { $$ = new mml::block_node(LINE, nullptr, nullptr);                }
               ;

declaration    : opt_qualifier data_type tIDENTIFIER opt_initializer ';'  { $$ = new mml::declaration_node(LINE, $1, $2, *$3, $4); }
               | opt_qualifier tTYPE_AUTO tIDENTIFIER opt_initializer ';' { $$ = new mml::declaration_node(LINE, $1, nullptr, *$3, $4); } // FIXME
               | opt_qualifier tIDENTIFIER opt_initializer ';'            { $$ = new mml::declaration_node(LINE, $1, nullptr, *$2, $3); }
               ;

initializer    : '=' expression                                      { $$ = $2;                                                         }
               ;

opt_initializer : initializer                                        { $$ = $1;                                                         }
				| /* empty */                                        { $$ = nullptr;                                                    }
                ;

qualifier      : tPUBLIC                                             { $$ = tPUBLIC;                                                    }
			   | tFOREIGN                                            { $$ = tFOREIGN;                                                   }
			   | tFORWARD                                            { $$ = tFORWARD;                                                   }
			   | /* empty */                                         { $$ = tPUBLIC;                                                    }
			   ;

opt_qualifier  : qualifier                                           { $$ = $1;                                                         }
			   | /* empty */                                         { $$ = tPUBLIC;                                                    }
			   ;

declarations   : /* empty */  declaration                            { $$ = new cdk::sequence_node(LINE, $1);                           }
               | declarations declaration                            { $$ = new cdk::sequence_node(LINE, $2, $1);                       }
               ;

instruction    : expression ';'                                      { $$ = new mml::evaluation_node(LINE, $1);                         }
               | expressions tPRINT                                  { $$ = new mml::print_node(LINE, $1, false);                       }
               | expressions tPRINTLN                                { $$ = new mml::print_node(LINE, $1, true);                        }
               | tSTOP ';'                                           { $$ = new mml::stop_node(LINE);                                   }
               | tSTOP tINTEGER ';'                                  { $$ = new mml::stop_node(LINE, $2);                               }
               | tNEXT ';'                                           { $$ = new mml::next_node(LINE);                                   }
               | tNEXT tINTEGER ';'                                  { $$ = new mml::next_node(LINE, $2);                               }
               | tRETURN ';'                                         { $$ = new mml::return_node(LINE, nullptr);                        }
               | tRETURN expression ';'                              { $$ = new mml::return_node(LINE, $2);                             }
               | tIF if_instruction                                  { $$ = $2;                                                         }
               | tWHILE '(' expression ')' instruction               { $$ = new mml::while_node(LINE, $3, $5);                          }
               | '{' block '}'                                       { $$ = $2;                                                         }
               ;

if_instruction : '(' expression ')' instruction                      { $$ = new mml::if_node(LINE, $2, $4);                             }
               | '(' expression ')' instruction tELSE instruction    { $$ = new mml::if_else_node(LINE, $2, $4, $6);                    }
               | '(' expression ')' instruction tELIF if_instruction { $$ = new mml::if_else_node(LINE, $2, $4, $6);                    }
               ;

instructions   : /* empty */  instruction                            { $$ = new cdk::sequence_node(LINE, $1);                           }
               | instructions instruction                            { $$ = new cdk::sequence_node(LINE, $2, $1);                       }
               ;

data_type      : tTYPE_INT                                           { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT);              }
               | tTYPE_DOUBLE                                        { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);           }
               | tTYPE_STRING                                        { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING);           }
               | tTYPE_VOID                                          { /* TODO */ }
               | '[' data_type ']'                                   { /* TODO */ }
               | function_type                                       { $$ = $1;                                                         }
               ;

opt_data_type  : data_type                                           { $$ = $1;                                                         }
			   | /* empty */                                         { $$ = nullptr;                                                    }
			   ;

function_type  : data_type '<' opt_data_type '>'                     { /* TODO */ }
			   ;

expression     : tINTEGER                                            { $$ = new cdk::integer_node(LINE, $1);                            }
               | tDOUBLE                                             { $$ = new cdk::double_node(LINE, $1);                             }
               | string                                              { $$ = new cdk::string_node(LINE, $1);                             }
               | tNULL                                               { $$ = new mml::null_node(LINE);                                   }
               | lvalue                                              { $$ = new cdk::rvalue_node(LINE, $1);                             }
               | lvalue '=' expression                               { $$ = new cdk::assignment_node(LINE, $1, $3);                     }
               | expression '+' expression                           { $$ = new cdk::add_node(LINE, $1, $3);                            }
               | expression '-' expression                           { $$ = new cdk::sub_node(LINE, $1, $3);                            }
               | expression '*' expression                           { $$ = new cdk::mul_node(LINE, $1, $3);                            }
               | expression '/' expression                           { $$ = new cdk::div_node(LINE, $1, $3);                            }
               | expression '%' expression                           { $$ = new cdk::mod_node(LINE, $1, $3);                            }
               | expression  '<' expression                          { $$ = new cdk::lt_node(LINE, $1, $3);                             }
               | expression tLE  expression                          { $$ = new cdk::le_node(LINE, $1, $3);                             }
               | expression tEQ  expression                          { $$ = new cdk::eq_node(LINE, $1, $3);                             }
               | expression tGE  expression                          { $$ = new cdk::ge_node(LINE, $1, $3);                             }
               | expression  '>' expression                          { $$ = new cdk::gt_node(LINE, $1, $3);                             }
               | expression tNE  expression                          { $$ = new cdk::ne_node(LINE, $1, $3);                             }
               | expression tAND  expression                         { $$ = new cdk::and_node(LINE, $1, $3);                            }
               | expression tOR  expression                          { $$ = new cdk::or_node (LINE, $1, $3);                            }
               | '-' expression %prec tUNARY                         { $$ = new cdk::neg_node(LINE, $2);                                }
               | '+' expression %prec tUNARY                         { $$ = new mml::identity_node(LINE, $2);                           }
               | '~' expression                                      { $$ = new cdk::not_node(LINE, $2);                                }
               | tREAD                                               { $$ = new mml::read_node(LINE);                                   }
               | tSIZEOF '(' expression ')'                          { $$ = new mml::sizeof_node(LINE, $3);                             }
               | '(' expression ')'                                  { $$ = $2;                                                         }
               | '[' expression ']'                                  { $$ = new mml::stack_alloc_node(LINE, $2);                        }
               | lvalue '?'                                          { $$ = new mml::address_of_node(LINE, $1);                         }
               | '(' ')' '-''>' data_type block                      { $$ = new mml::function_definition_node(LINE, nullptr, $5, $6);   }
               | '(' variables ')' '-''>' data_type block            { $$ = new mml::function_definition_node(LINE, $2, $6, $7);        }
               | '(' variables ')' '-''>' data_type block            { $$ = new mml::function_definition_node(LINE, $2, $6, $7);        }
               | expression '(' ')'                                  { $$ = new mml::function_call_node(LINE, $1, nullptr);             }
               | expression '(' expressions ')'                      { $$ = new mml::function_call_node(LINE, $1, $3);                  }
               ;

expressions    : expression                                          { $$ = new cdk::sequence_node(LINE, $1);                           }
               | expressions ',' expression                          { $$ = new cdk::sequence_node(LINE, $3, $1);                       }
               ;

variable       : opt_qualifier data_type tIDENTIFIER                 { $$ = new mml::declaration_node(LINE, $1, $2, *$3, nullptr);      }
               ;

variables 	   : /* empty */                                         { $$ = new cdk::sequence_node(LINE);                               }
			   | variables ',' variable                              { $$ = new cdk::sequence_node(LINE, $3, $1);                       }
			   ;

string         : tSTRING                                             { $$ = $1;                                                         }
               | string tSTRING                                      { $$ = $1; $$->append(*$2); delete $2;                             }
               ;

lvalue         : tIDENTIFIER                                         { $$ = new cdk::variable_node(LINE, $1);                           }
               ;

%%
