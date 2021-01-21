%skeleton "lalr1.cc"
%require "3.7.1"
%defines

%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {
    #include <string>

    class ParseDriver;
}

%param { ParseDriver& drv }
%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
    #include "ParseDriver/ParseDriver.hpp"
}

%define api.token.prefix {TOK_}
%token
    ASSIGN  "="
    PLUS    "+"
    MINUS   "-"
    MULT    "*"
    DIV     "/"
    LPAREN  "("
    RPAREN  ")"
    SEMI    ";"
    ;

%token <std::string> IDENTIFIER "identifier"
%token <int> NUMBER "number"
%nterm <int> exp

%printer { yyo << $$; } <*>;

%%
%start stmts;

stmts:
    %empty
    | stmts stmt
    ;

stmt:
    "identifier" "=" exp { drv.variables[$1] = $3; }
;

%left "+" "-";
%left "*" "/";
exp:
    "identifier"    { $$ = drv.variables[$1]; }
|   "number"
|   exp "+" exp   { $$ = $1 + $3; }
|   exp "-" exp   { $$ = $1 - $3; }
|   exp "*" exp   { $$ = $1 * $3; }
|   exp "/" exp   { $$ = $1 / $3; }
|   "(" exp ")"     { $$ = $2; }
;


%%

void yy::parser::error(const location_type& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}