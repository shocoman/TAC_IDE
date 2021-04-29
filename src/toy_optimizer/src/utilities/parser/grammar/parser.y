%skeleton "lalr1.cc"
%require "3.5.1"
%defines

%define api.parser.class { Parser }
%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include <string>
    struct ParseDriver;
    #include "../../../structure/quadruple/quadruple.hpp"
}

%code {
    #include "../parser/driver/driver.hpp"
    #include <fmt/ranges.h>
}

%param { ParseDriver& drv }

%locations
%define parse.trace
%define parse.error verbose
%define parse.lac full

%define api.token.prefix {TOK_}
%token
    EOF 0 "end of file"

    IFTRUE   "if"
    IFFALSE  "iffalse"
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
    AND     "&&"
    OR      "||"
    NOT     "!"
    XOR     "^"
    MODULUS "%"

    CMP_LT  "<"
    CMP_LTE "<="
    CMP_GT  ">"
    CMP_GTE ">="
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
%token <std::string> BOOL "bool"
%token <char> CHAR "char"
%token <int> INT "int"
%token <double> FLOAT "float"


%nterm <Dest> dest
%nterm <Quad> value quadruple if_statement goto assignment array_assignment var_declaration
%nterm <std::string> label
%nterm <Operand> term

%left "||" "&&" "^"
%left "<" ">" "==" "!="
%left "+" "-";
%left "*" "/" "%";

// %printer { yyo << $$; } <*>;
%start program;
%%

program:
    %empty
|   program statement
;

statement:
    label mb_newline       { drv.labels.emplace($label, drv.quadruples.size()); }
|   quadruple newlines     { drv.quadruples.push_back($quadruple); }
;
mb_newline: %empty | "newline";
newlines: "end of file" | "newline" | newlines "newline";

quadruple:
    assignment
|   if_statement
|   goto
|   "halt"                              { $$ = Quad({}, {}, Quad::Type::Halt); };
|   "nop"                               { $$ = Quad({}, {}, Quad::Type::Nop); };
|   "return" term                       { $$ = Quad($term, {}, Quad::Type::Return); };
|   "print_to_console"  term            { $$ = Quad($term, {}, Quad::Type::Print); };
|   "call"  "identifier"[id] "," "int"  {
         $$ = Quad(Operand($id, Operand::Type::None), Operand(std::to_string($4), Operand::Type::None), Quad::Type::Call);
       };
|   var_declaration
|   "putparam" term                     { $$ = Quad($term, {}, Quad::Type::Putparam); };
|   "getparam" "identifier"[id]         { $$ = Quad($id, {}, Quad::Type::Getparam); };
;

var_declaration:
    label "." "identifier"[id] term[initval]
                              {
                                  $$ = Quad(Operand($id, Operand::Type::None), $initval, Quad::Type::VarDeclaration);
                                  $$.dest = Dest($label, Dest::Type::Var);
                              };
|   label "." "block" term[size] "," "identifier"[type] "," term[initval]
                              {
                                  $$ = Quad($size, $initval, Quad::Type::ArrayDeclaration);
                                  $$.ops.push_back(Operand($type, Operand::Type::None));
                                  $$.dest = Dest($label, Dest::Type::Array);
                              };

assignment:
    dest "=" value { $value.dest = $dest; $$ = $value; };
|   array_assignment
;

array_assignment: "identifier" "[" term[i] "]" "=" term[rhs]   { $$ = Quad($i, $rhs, Quad::Type::ArraySet);
                                                                   $$.dest = Dest($1, Dest::Type::Array); };


if_statement:
    "if" term "goto" "identifier"[id]        { $$ = Quad($term, {}, Quad::Type::IfTrue);
                                                $$.dest = Dest($id, Dest::Type::JumpLabel); }
|   "iffalse" term "goto" "identifier"[id]   { $$ = Quad($term, {}, Quad::Type::IfFalse);
                                                $$.dest = Dest($id, Dest::Type::JumpLabel);}
;

goto: "goto" "identifier"[id]   { $$ = Quad({}, {}, Quad::Type::Goto);
                                $$.dest = Dest($id, Dest::Type::JumpLabel);};

dest:
    "identifier"                { $$ = Dest($1, Dest::Type::Var); }
|   "*" "identifier"            { $$ = Dest($2, Dest::Type::Deref); }
;

label:
    "identifier" ":" { $$ = $1; }
;

value:
    term                    { $$ = Quad($1, {}, Quad::Type::Assign); }
|   "*" term                { $$ = Quad($2, {}, Quad::Type::Deref); }
|   "-" term                { $$ = Quad($2, {}, Quad::Type::UMinus); }
|   "&" term                { $$ = Quad($2, {}, Quad::Type::Ref); }
|   "!" term                { $$ = Quad($2, {}, Quad::Type::Not); }
|   term "+"  term          { $$ = Quad($1, $3, Quad::Type::Add); }
|   term "-"  term          { $$ = Quad($1, $3, Quad::Type::Sub); }
|   term "*"  term          { $$ = Quad($1, $3, Quad::Type::Mult); }
|   term "&&"  term         { $$ = Quad($1, $3, Quad::Type::And); }
|   term "||"  term         { $$ = Quad($1, $3, Quad::Type::Or); }
|   term "^"  term          { $$ = Quad($1, $3, Quad::Type::Xor); }
|   term "/"  term          { $$ = Quad($1, $3, Quad::Type::Div); }
|   term "%"  term          { $$ = Quad($1, $3, Quad::Type::Modulus); }
|   term "<"  term          { $$ = Quad($1, $3, Quad::Type::Lt); }
|   term "<="  term         { $$ = Quad($1, $3, Quad::Type::Lte); }
|   term ">"  term          { $$ = Quad($1, $3, Quad::Type::Gt); }
|   term ">="  term         { $$ = Quad($1, $3, Quad::Type::Gte); }
|   term "==" term          { $$ = Quad($1, $3, Quad::Type::Eq); }
|   term "!=" term          { $$ = Quad($1, $3, Quad::Type::Neq); }
|   term "[" term "]"       {
                             $1.type = Operand::Type::Array;
                             $$ = Quad($1, $3, Quad::Type::ArrayGet); }
|   "call"  "identifier"[id] "," "int"  {
        $$ = Quad(Operand($id, Operand::Type::None), Operand(std::to_string($4), Operand::Type::None), Quad::Type::Call);
       }
;

term:
    "identifier"    { $$ = Operand($1, Operand::Type::Var); }
|   "string"        { $$ = Operand($1, Operand::Type::LString); }
|   "char"          { $$ = Operand(fmt::format("{}", $1), Operand::Type::LChar); }
|   "int"           { $$ = Operand(fmt::format("{}", $1), Operand::Type::LInt); }
|   "float"         { $$ = Operand(fmt::format("{}", $1), Operand::Type::LDouble); }
|   "bool"          { $$ = Operand($1, Operand::Type::LBool); }
;

%%

#include <sstream>
std::string g_bison_error_msg;
void yy::Parser::error(const location_type& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;

    std::stringstream s;
    s << l << ": " << m;
    g_bison_error_msg = s.str();
}