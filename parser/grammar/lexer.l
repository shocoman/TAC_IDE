%{
    # include <cerrno>
    # include <climits>
    # include <cstdlib>
    # include <cstring>
    # include <string>
    #include "ParseDriver/ParseDriver.hpp"
    #include "parser.hpp"
%}

%option noyywrap nounput noinput batch debug

%{

    yy::parser::symbol_type make_NUMBER(const std::string& s, const yy::parser::location_type& loc);
    #define YY_USER_ACTION loc.columns(yyleng);
%}


id  [a-zA-Z$_][a-zA-Z_0-9]*
int [0-9]+
blank [ \t\r]

%%

%{

    yy::location& loc = drv.location;
    loc.step();

%}

{blank}+    loc.step();
\n+         loc.lines(yyleng);; loc.step();


"="     return yy::parser::make_ASSIGN(loc);
"+"     return yy::parser::make_PLUS(loc);
"-"     return yy::parser::make_MINUS(loc);
"*"     return yy::parser::make_MULT(loc);
"/"     return yy::parser::make_DIV(loc);
";"     return yy::parser::make_SEMI(loc);
"("     return yy::parser::make_LPAREN(loc);
")"     return yy::parser::make_RPAREN(loc);

{int}   return make_NUMBER(yytext, loc);
{id}    return yy::parser::make_IDENTIFIER(yytext, loc);

.       {
            throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext));
        }


<<EOF>> return yy::parser::make_YYEOF(loc);

%%

yy::parser::symbol_type make_NUMBER(const std::string& s, const yy::parser::location_type& loc) {
    errno = 0;
    long n = strtol(s.c_str(), NULL, 10);
    if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
        throw yy::parser::syntax_error(loc, "integer is out of range: " + s);

    return yy::parser::make_NUMBER((int)n, loc);
}


void ParseDriver::scan_begin()
{
    yy_flex_debug = trace_scanning;
    if (file.empty() || file == "-")
        yyin = stdin;
    else if (!(yyin = fopen(file.c_str(), "r")))
    {
        std::cerr << "cannot open " << file << ": " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ParseDriver::scan_end() {
    fclose(yyin);
}