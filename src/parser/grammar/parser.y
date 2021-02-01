%skeleton "lalr1.cc"
%require "3.7.1"
%defines

%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include <string>
    struct ParseDriver;
    #include "../../src/tac_worker/structure/quadruple/quadruple.hpp"
}

%param { ParseDriver& drv }
%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
    #include "../parser/driver/driver.hpp"
}

%define api.token.prefix {TOK_}
%token
    IFTRUE   "if"
    IFFALSE  "ifFalse"
    GOTO     "goto"
    HALT     "halt"
    CALL     "call"
    PUTPARAM "putparam"
    GETPARAM "getparam"
    NOP      "nop"
    RETURN   "return"
    PRINT    "print"
    NEWLINE  "newline"
    BLOCK    "block"

    ASSIGN  "="
    PLUS    "+"
    MINUS   "-"
    MULT    "*"
    DIV     "/"
    REF     "&"

    CMP_LT  "<"
    CMP_GT  ">"
    CMP_EQ  "=="
    CMP_NEQ "!="

    LPAREN   "("
    RPAREN   ")"
    LBRACKET "["
    RBRACKET "]"
    SEMI     ";"
    COLON    ":"
    COMMA    ","
    DOT      "."
    ;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> STRING "string"
%token <char> CHAR "char"
%token <int> INT "int"
%token <double> FLOAT "float"


%nterm <Dest> dest
%nterm <Quad> value quadruple if_statement goto assignment var_declaration
%nterm <std::string> label
%nterm <Operand> term

%left "<" ">" "==" "!="
%left "+" "-";
%left "*" "/";

// %printer { yyo << $$; } <*>;
%%
%start stmts;

stmts:
    %empty
|   stmts stmt
;

stmt:
    label mb_newline       { drv.labels.emplace($label, drv.quadruples.size()); }
|   quadruple newlines     { drv.quadruples.push_back($quadruple); }
;
mb_newline: %empty | "newline";
newlines: YYEOF | "newline" | newlines "newline";

quadruple:
    assignment
|   if_statement
|   goto
|   "halt"                              { $$ = Quad({}, {}, Quad::Type::Halt); };
|   "nop"                               { $$ = Quad({}, {}, Quad::Type::Nop); };
|   "return" term                       { $$ = Quad($term, {}, Quad::Type::Return); };
|   "print_to_console"  term            { $$ = Quad($term, {}, Quad::Type::Print); };
|   "call"  "identifier"[id] "," "int"  { $$ = Quad($id, std::to_string($4), Quad::Type::Call); };
|   var_declaration
|   "putparam" term                     { $$ = Quad($term, {}, Quad::Type::Putparam); };
|   "getparam" "identifier"[id]         { $$ = Quad({}, {}, Quad::Type::Getparam);
                                          $$.dest = Dest($id, {}, Dest::Type::Var);  };
;

var_declaration:
    label "." "identifier"[id] term {
                                  $$ = Quad($id, $term, Quad::Type::VarDeclaration);
                                  $$.dest = Dest($label, {}, Dest::Type::Var);
                              };
|   label "." "block" "int"[size] "," "identifier"[type] "," term[initval] {
                                  $$ = Quad($type, $initval, Quad::Type::ArrayDeclaration);
                                  $$.dest = Dest($label, std::to_string($size), Dest::Type::Var);
                              };

assignment: dest "=" value { $value.dest = $dest; $$ = $value; };

if_statement:
    "if" term "goto" "identifier"[id]        { $$ = Quad($term, {}, Quad::Type::IfTrue);
                                                $$.dest = Dest($id, {}, Dest::Type::JumpLabel); }
|   "ifFalse" term "goto" "identifier"[id]   { $$ = Quad($term, {}, Quad::Type::IfFalse);
                                                $$.dest = Dest($id, {}, Dest::Type::JumpLabel);}
;

goto: "goto" "identifier"[id]   { $$ = Quad({}, {}, Quad::Type::Goto);
                                $$.dest = Dest($id, {}, Dest::Type::JumpLabel);};

dest:
    "identifier"                { $$ = Dest($1, {}, Dest::Type::Var); }
|   "*" "identifier"            { $$ = Dest($2, {}, Dest::Type::Deref); }
|   "identifier" "[" term "]"   { $$ = Dest($1, $term, Dest::Type::ArraySet); }

label:
    "identifier" ":" { $$ = $1; }
;

value:
    term                    { $$ = Quad($1, {}, Quad::Type::Assign); }
|   "*" term                { $$ = Quad($2, {}, Quad::Type::Deref); }
|   "-" term                { $$ = Quad($2, {}, Quad::Type::UMinus); }
|   "&" term                { $$ = Quad($2, {}, Quad::Type::Ref); }
|   term "[" term "]"       { $$ = Quad($1, $3, Quad::Type::ArrayGet); }
|   term "+"  term          { $$ = Quad($1, $3, Quad::Type::Add); }
|   term "-"  term          { $$ = Quad($1, $3, Quad::Type::Sub); }
|   term "*"  term          { $$ = Quad($1, $3, Quad::Type::Mult); }
|   term "/"  term          { $$ = Quad($1, $3, Quad::Type::Div); }
|   term "<"  term          { $$ = Quad($1, $3, Quad::Type::Lt); }
|   term ">"  term          { $$ = Quad($1, $3, Quad::Type::Gt); }
|   term "==" term          { $$ = Quad($1, $3, Quad::Type::Eq); }
|   term "!=" term          { $$ = Quad($1, $3, Quad::Type::Neq); }
|   "call"  "identifier"[id] "," "int"  { $$ = Quad($id, std::to_string($4), Quad::Type::Call); };
;

term:
    "identifier"    { $$ = Operand($1, Operand::Type::Var); }
|   "string"        { $$ = Operand($1, Operand::Type::LString); }
|   "char"          { $$ = Operand(std::string(1, $1), Operand::Type::LChar); }
|   "int"           { $$ = Operand(std::to_string($1), Operand::Type::LInt); }
|   "float"         { $$ = Operand(std::to_string($1), Operand::Type::LDouble); }
;

%%

void yy::parser::error(const location_type& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}