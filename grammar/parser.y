%skeleton "lalr1.cc"
%require "3.7.1"
%defines

%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include <string>


    struct driver;

    #include "../tac_worker/quadruple.hpp"


}

%param { driver& drv }
%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
    #include "driver.hpp"
}

%define api.token.prefix {TOK_}
%token
    IFTRUE  "if"
    IFFALSE "ifFalse"
    GOTO    "goto"
    HALT    "halt"
    CALL    "call"
    PARAM   "param"
    NOP     "nop"
    NEWLINE "newline"

    ASSIGN  "="
    PLUS    "+"
    MINUS   "-"
    MULT    "*"
    DIV     "/"

    CMP_LT  "<"
    CMP_GT  ">"
    CMP_EQ  "=="
    CMP_NEQ "!="

    LPAREN  "("
    RPAREN  ")"
    LBRACKET "["
    RBRACKET "]"
    SEMI    ";"
    COLON   ":"
    ;

%token <std::string> IDENTIFIER "identifier"
%token <int> INT "int"
%token <double> FLOAT "float"


%nterm <Destination> dest
%nterm <Quadruple> value quadruple if_statement goto assignment
%nterm <std::string> term label
//%nterm <std::vector<Quadruple>> stmts

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
    label mb_newline               { drv.labels.emplace($label, drv.quadruples.size()); }
|   quadruple newlines     { drv.quadruples.push_back($quadruple); }
;
mb_newline: %empty | "newline";
newlines: YYEOF | "newline" | newlines "newline";


quadruple:
    assignment
|   if_statement
|   goto
|   "halt"                          { $$ = Quadruple({}, {}, OperationType::Halt); };
|   "param" term                    { $$ = Quadruple($term, {}, OperationType::Param); };
|   "call"  "identifier"[id] term   { $$ = Quadruple($id, $term, OperationType::Call); };
|   "nop"  "identifier"[id] term    { $$ = Quadruple($id, $term, OperationType::Nop); };
;

assignment: dest "=" value { $value.dest = $dest; $$ = $value; };

if_statement:
    "if" term "goto" "identifier"[id]        { $$ = Quadruple($term, {}, OperationType::IfTrue);
                                                $$.dest = Destination($id, {}, DestinationType::JumpLabel); }
|   "ifFalse" term "goto" "identifier"[id]   { $$ = Quadruple($term, {}, OperationType::IfFalse);
                                                $$.dest = Destination($id, {}, DestinationType::JumpLabel);}
;

goto: "goto" "identifier"[id]   { $$ = Quadruple({}, {}, OperationType::Goto);
                                $$.dest = Destination($id, {}, DestinationType::JumpLabel);};



dest:
    "identifier"                { $$ = Destination($1, {}, DestinationType::Var); }
|   "*" "identifier"            { $$ = Destination($2, {}, DestinationType::Deref); }
|   "identifier" "[" term "]"   { $$ = Destination($1, $term, DestinationType::ArraySet); }

label:
    "identifier" ":" { $$ = $1; }
;


value:
    term                    { $$ = Quadruple($1, {}, OperationType::Copy); }
|   "*" term                { $$ = Quadruple($2, {}, OperationType::Deref); }
|   "-" term                { $$ = Quadruple($2, {}, OperationType::UMinus); }
|   term "[" term "]"       { $$ = Quadruple($1, $3, OperationType::ArrayGet); }
|   term "+"  term          { $$ = Quadruple($1, $3, OperationType::Add); }
|   term "-"  term          { $$ = Quadruple($1, $3, OperationType::Sub); }
|   term "*"  term          { $$ = Quadruple($1, $3, OperationType::Mult); }
|   term "/"  term          { $$ = Quadruple($1, $3, OperationType::Div); }
|   term "<"  term          { $$ = Quadruple($1, $3, OperationType::Lt); }
|   term ">"  term          { $$ = Quadruple($1, $3, OperationType::Gt); }
|   term "==" term          { $$ = Quadruple($1, $3, OperationType::Eq); }
|   term "!=" term          { $$ = Quadruple($1, $3, OperationType::Neq); }
;

term:
    "identifier"    { $$ = $1; }
|   "int"           { $$ = std::to_string($1); }
|   "float"         { $$ = std::to_string($1); }
;


%%

void yy::parser::error(const location_type& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}