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

  std::vector<std::shared_ptr<cdk::basic_type>> *types;
};

%token <i> tINTEGER tQUALIFIER
%token <d> tDOUBLE
%token <s> tIDENTIFIER tSTRING
%token tTYPE_INT tTYPE_DOUBLE tTYPE_STRING tTYPE_VOID tTYPE_AUTO
%token tPUBLIC tFORWARD tFOREIGN tPRIVATE
%token tIF tELIF tELSE tWHILE tSTOP tNEXT tRETURN
%token tPRINT tPRINTLN tREAD tNULL tSIZEOF
%token tBEGIN tEND
%token tAND tOR tNE tLE tGE
%token tARROW

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

%type <node> file instruction if_instruction global_declaration non_global_declaration variable
%type <sequence> instructions global_declarations non_global_declarations expressions variables
%type <expression> expression function literal initializer opt_initializer literal_initializer opt_literal_initializer
%type <block> block
%type <lvalue> lvalue
%type <type> data_type
%type <types> data_types
%type <s> string

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file                    : global_declarations                                          { compiler->ast(new mml::program_node(LINE, $1, nullptr));                                                  }
                        | tBEGIN block tEND                                            { compiler->ast(new mml::program_node(LINE, new cdk::sequence_node(LINE), $2));                             }
                        | global_declarations tBEGIN block tEND                        { compiler->ast(new mml::program_node(LINE, $1, $3));                                                       }
                        ;

block                   : non_global_declarations instructions                         { $$ = new mml::block_node(LINE, $1, $2);                                                                   }
                        | non_global_declarations                                      { $$ = new mml::block_node(LINE, $1, new cdk::sequence_node(LINE));                                         }
                        | instructions                                                 { $$ = new mml::block_node(LINE, new cdk::sequence_node(LINE), $1);                                         }
                        | /* empty */                                                  { $$ = new mml::block_node(LINE, new cdk::sequence_node(LINE), new cdk::sequence_node(LINE));               }
                        ;

global_declaration      : tPUBLIC             tIDENTIFIER opt_literal_initializer ';'  { 
                                                                                         $$ = new mml::declaration_node(LINE, tPUBLIC, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC), *$2, $3); 
                                                                                         delete $2;  
                                                                                       }
                        | tPUBLIC  tTYPE_AUTO tIDENTIFIER literal_initializer ';'      { 
                                                                                         $$ = new mml::declaration_node(LINE, tPUBLIC, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC), *$3, $4); 
                                                                                         delete $3;  
                                                                                       }
                        | tPUBLIC  data_type  tIDENTIFIER opt_literal_initializer ';'  { $$ = new mml::declaration_node(LINE, tPUBLIC, $2, *$3, $4); delete $3;                                    }
                        | tFORWARD data_type  tIDENTIFIER ';'                          { $$ = new mml::declaration_node(LINE, tFORWARD, $2, *$3, nullptr); delete $3;                              }
                        | tFOREIGN data_type  tIDENTIFIER ';'                          { $$ = new mml::declaration_node(LINE, tFOREIGN, $2, *$3, nullptr); delete $3;                              }
                        | data_type  tIDENTIFIER opt_literal_initializer ';'           { $$ = new mml::declaration_node(LINE, tPRIVATE, $1, *$2, $3); delete $2;                                   }
                        | tTYPE_AUTO tIDENTIFIER literal_initializer ';'               { 
                                                                                         $$ = new mml::declaration_node(LINE, tPRIVATE, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC), *$2, $3); 
                                                                                         delete $2; 
                                                                                       }

non_global_declaration  : data_type  tIDENTIFIER opt_initializer ';'                   { $$ = new mml::declaration_node(LINE, tPRIVATE, $1, *$2, $3); delete $2;                                   }
                        | tTYPE_AUTO tIDENTIFIER initializer ';'                       { 
                                                                                         $$ = new mml::declaration_node(LINE, tPRIVATE, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC), *$2, $3); 
                                                                                         delete $2; 
                                                                                       }
                        ;

initializer             : '=' expression                                               { $$ = $2;                                                                                                  }
                        ;

opt_initializer         : /* empty */                                                  { $$ = nullptr;                                                                                             }
                        | initializer                                                  { $$ = $1;                                                                                                  }
                        ;

literal_initializer     : '=' literal                                                  { $$ = $2;                                                                                                  }
                        ;

opt_literal_initializer : /* empty */                                                  { $$ = nullptr;                                                                                             }
                        | literal_initializer                                          { $$ = $1;                                                                                                  }
                        ;

global_declarations     : /* empty */    global_declaration                            { $$ = new cdk::sequence_node(LINE, $1);                                                                    }
                        | global_declarations global_declaration                       { $$ = new cdk::sequence_node(LINE, $2, $1);                                                                }
                        ;

non_global_declarations : /* empty */    non_global_declaration                        { $$ = new cdk::sequence_node(LINE, $1);                                                                    }
                        | non_global_declarations non_global_declaration               { $$ = new cdk::sequence_node(LINE, $2, $1);                                                                }
                        ;

instruction             : expression ';'                                               { $$ = new mml::evaluation_node(LINE, $1);                                                                  }
                        | expressions tPRINT                                           { $$ = new mml::print_node(LINE, $1, false);                                                                }
                        | expressions tPRINTLN                                         { $$ = new mml::print_node(LINE, $1, true);                                                                 }
                        | tSTOP ';'                                                    { $$ = new mml::stop_node(LINE);                                                                            }
                        | tSTOP tINTEGER ';'                                           { $$ = new mml::stop_node(LINE, $2);                                                                        }
                        | tNEXT ';'                                                    { $$ = new mml::next_node(LINE);                                                                            }
                        | tNEXT tINTEGER ';'                                           { $$ = new mml::next_node(LINE, $2);                                                                        }
                        | tRETURN ';'                                                  { $$ = new mml::return_node(LINE, nullptr);                                                                 }
                        | tRETURN expression ';'                                       { $$ = new mml::return_node(LINE, $2);                                                                      }
                        | tIF if_instruction                                           { $$ = $2;                                                                                                  }
                        | tWHILE '(' expression ')' instruction                        { $$ = new mml::while_node(LINE, $3, $5);                                                                   }
                        | '{' block '}'                                                { $$ = $2;                                                                                                  }
                        ;

if_instruction          : '(' expression ')' instruction                               { $$ = new mml::if_node(LINE, $2, $4);                                                                      }
                        | '(' expression ')' instruction tELSE instruction             { $$ = new mml::if_else_node(LINE, $2, $4, $6);                                                             }
                        | '(' expression ')' instruction tELIF if_instruction          { $$ = new mml::if_else_node(LINE, $2, $4, $6);                                                             }
                        ;

instructions            : /* empty */  instruction                                     { $$ = new cdk::sequence_node(LINE, $1);                                                                    }
                        | instructions instruction                                     { $$ = new cdk::sequence_node(LINE, $2, $1);                                                                }
                        ;

data_type               : tTYPE_INT                                                    { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT);                                                       }
                        | tTYPE_DOUBLE                                                 { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE);                                                    }
                        | tTYPE_STRING                                                 { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING);                                                    }
                        | '[' data_type ']'                                            { $$ = cdk::reference_type::create(4, $2);                                                                  }
                        | '[' tTYPE_VOID ']'                                           { $$ = cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_VOID));                      }
                        | data_type '<' data_types '>'                                 {
                                                                                         auto output = new std::vector<std::shared_ptr<cdk::basic_type>>();
                                                                                         output->push_back($1);
                                                                                         $$ = cdk::functional_type::create(*$3, *output);
                                                                                         delete $3;
                                                                                         delete output;
                                                                                       }
                        | tTYPE_VOID '<' data_types '>'                                {
                                                                                         auto output = new std::vector<std::shared_ptr<cdk::basic_type>>();
                                                                                         output->push_back(cdk::primitive_type::create(0, cdk::TYPE_VOID));
                                                                                         $$ = cdk::functional_type::create(*$3, *output);
                                                                                         delete $3;
                                                                                         delete output;
                                                                                       }
                        ;

data_types              : /* empty */                                                  { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>();                                                 }
                        | data_type                                                    { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); $$->push_back($1);                              }
                        | data_types ',' data_type                                     { $$ = $1; $$->push_back($3);                                                                               }
                        ;

expression              : tINTEGER                                                     { $$ = new cdk::integer_node(LINE, $1);                                                                     }
                        | tDOUBLE                                                      { $$ = new cdk::double_node(LINE, $1);                                                                      }
                        | string                                                       { $$ = new cdk::string_node(LINE, $1);                                                                      }
                        | tNULL                                                        { $$ = new mml::null_node(LINE);                                                                            }
                        | lvalue                                                       { $$ = new cdk::rvalue_node(LINE, $1);                                                                      }
                        | lvalue '=' expression                                        { $$ = new cdk::assignment_node(LINE, $1, $3);                                                              }
                        | expression '+' expression                                    { $$ = new cdk::add_node(LINE, $1, $3);                                                                     }
                        | expression '-' expression                                    { $$ = new cdk::sub_node(LINE, $1, $3);                                                                     }
                        | expression '*' expression                                    { $$ = new cdk::mul_node(LINE, $1, $3);                                                                     }
                        | expression '/' expression                                    { $$ = new cdk::div_node(LINE, $1, $3);                                                                     }
                        | expression '%' expression                                    { $$ = new cdk::mod_node(LINE, $1, $3);                                                                     }
                        | expression '<' expression                                    { $$ = new cdk::lt_node(LINE, $1, $3);                                                                      }
                        | expression tLE expression                                    { $$ = new cdk::le_node(LINE, $1, $3);                                                                      }
                        | expression tEQ expression                                    { $$ = new cdk::eq_node(LINE, $1, $3);                                                                      }
                        | expression tGE expression                                    { $$ = new cdk::ge_node(LINE, $1, $3);                                                                      }
                        | expression '>' expression                                    { $$ = new cdk::gt_node(LINE, $1, $3);                                                                      }
                        | expression tNE expression                                    { $$ = new cdk::ne_node(LINE, $1, $3);                                                                      }
                        | expression tAND expression                                   { $$ = new cdk::and_node(LINE, $1, $3);                                                                     }
                        | expression tOR expression                                    { $$ = new cdk::or_node (LINE, $1, $3);                                                                     }
                        | '-' expression %prec tUNARY                                  { $$ = new cdk::neg_node(LINE, $2);                                                                         }
                        | '+' expression %prec tUNARY                                  { $$ = $2;                                                                                                  }
                        | '~' expression                                               { $$ = new cdk::not_node(LINE, $2);                                                                         }
                        | tREAD                                                        { $$ = new mml::read_node(LINE);                                                                            }
                        | tSIZEOF '(' expression ')'                                   { $$ = new mml::sizeof_node(LINE, $3);                                                                      }
                        | '(' expression ')'                                           { $$ = $2;                                                                                                  }
                        | '[' expression ']'                                           { $$ = new mml::stack_alloc_node(LINE, $2);                                                                 }
                        | lvalue '?'                                                   { $$ = new mml::address_of_node(LINE, $1);                                                                  }
                        | function                                                     { $$ = $1;                                                                                                  }
                        | expression '(' ')'                                           { $$ = new mml::function_call_node(LINE, $1, new cdk::sequence_node(LINE));                                 }
                        | expression '(' expressions ')'                               { $$ = new mml::function_call_node(LINE, $1, $3);                                                           }
                        | '@' '(' expressions ')'                                      { $$ = new mml::function_call_node(LINE, nullptr, $3);                                                      }
                        | '@' '(' ')'                                                  { $$ = new mml::function_call_node(LINE, nullptr, new cdk::sequence_node(LINE));                            }
                        ;

expressions             : expression                                                   { $$ = new cdk::sequence_node(LINE, $1);                                                                    }
                        | expressions ',' expression                                   { $$ = new cdk::sequence_node(LINE, $3, $1);                                                                }
                        ;

variable                : data_type tIDENTIFIER                                        { $$ = new mml::declaration_node(LINE, tPRIVATE, $1, *$2, nullptr); delete $2;                              }
                        ;

variables               : /* empty */                                                  { $$ = new cdk::sequence_node(LINE);                                                                        }
                        | variable                                                     { $$ = new cdk::sequence_node(LINE, $1);                                                                    }
                        | variables ',' variable                                       { $$ = new cdk::sequence_node(LINE, $3, $1);                                                                }
                        ;

string                  : tSTRING                                                      { $$ = $1;                                                                                                  }
                        | string tSTRING                                               { $$ = $1; $$->append(*$2); delete $2;                                                                      }
                        ;

function                : '(' variables ')' tARROW data_type '{' block '}'             { $$ = new mml::function_definition_node(LINE, $2, $5, $7);                                                 }
                        | '(' variables ')' tARROW tTYPE_VOID '{' block '}'            { $$ = new mml::function_definition_node(LINE, $2, cdk::primitive_type::create(0, cdk::TYPE_VOID), $7);     }

lvalue                  : tIDENTIFIER                                                  { $$ = new cdk::variable_node(LINE, $1);                                                                    }
                        | expression '[' expression ']'                                { $$ = new mml::index_node(LINE, $1, $3);                                                                   }
                        ;

literal                 : tINTEGER                                                     { $$ = new cdk::integer_node(LINE, $1);                                                                     }
                        | tDOUBLE                                                      { $$ = new cdk::double_node(LINE, $1);                                                                      }
                        | string                                                       { $$ = new cdk::string_node(LINE, $1);                                                                      }
                        | tNULL                                                        { $$ = new mml::null_node(LINE);                                                                            }
                        | function                                                     { $$ = $1;                                                                                                  }
                        ;

%%
