%{
    //#pragma warning (disable : 4005)
    #include <climits>
    #include <cstring>
    #include <string>
    #include <functional>
    #include <map>
    #include "../driver/driver.hpp"
%}

%option noyywrap nounput noinput batch debug

%{
    yy::parser::symbol_type make_NUMBER(const std::string& s, const yy::parser::location_type& loc, bool);
    yy::parser::symbol_type make_IDENTIFIER(const std::string& s, const yy::parser::location_type& loc);
    #define YY_USER_ACTION loc.columns(yyleng);
%}


id  [a-zA-Z$_][a-zA-Z_.0-9]*
int [0-9]+
float {int}\.{int}?
string \"([^\\\"]|\\.)*\"
char '([^'\\\n]|\\.)'

blank [ \t\r]
line_comment "//".*
line_comment_with_nl ^{blank}*"//".*\n
empty_line ^{blank}*\n

%%

%{
    yy::location& loc = drv.location;
    loc.step();
%}

{empty_line}+               loc.lines(yyleng); loc.step();
\n                          loc.lines(yyleng); loc.step(); return yy::parser::make_NEWLINE(loc);
{blank}+                    loc.step();
{line_comment_with_nl}      loc.lines(yytext[yyleng-1] == '\n' ? 1 : 0); loc.step();
{line_comment}              loc.lines(yytext[yyleng-1] == '\n' ? 1 : 0); loc.step();

"="     return yy::parser::make_ASSIGN(loc);
"+"     return yy::parser::make_PLUS(loc);
"-"     return yy::parser::make_MINUS(loc);
"*"     return yy::parser::make_MULT(loc);
"/"     return yy::parser::make_DIV(loc);
"&"     return yy::parser::make_REF(loc);

"<"     return yy::parser::make_CMP_LT(loc);
">"     return yy::parser::make_CMP_GT(loc);
"=="    return yy::parser::make_CMP_EQ(loc);
"!="    return yy::parser::make_CMP_NEQ(loc);


"."     return yy::parser::make_DOT(loc);
","     return yy::parser::make_COMMA(loc);
";"     return yy::parser::make_SEMI(loc);
":"     return yy::parser::make_COLON(loc);
"("     return yy::parser::make_LPAREN(loc);
")"     return yy::parser::make_RPAREN(loc);
"["     return yy::parser::make_LBRACKET(loc);
"]"     return yy::parser::make_RBRACKET(loc);


{char}   return yy::parser::make_CHAR(yytext[1], loc);
{string} return yy::parser::make_STRING(yytext, loc);
{float}  return make_NUMBER(yytext, loc, false);
{int}    return make_NUMBER(yytext, loc, true);
{id}     return make_IDENTIFIER(yytext, loc);

.       {
            throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext));
        }


<<EOF>> return yy::parser::make_YYEOF(loc);
%%

yy::parser::symbol_type make_NUMBER(const std::string& s, const yy::parser::location_type& loc, bool is_int) {
    errno = 0;
    if (is_int) {
        long n = strtol(s.c_str(), NULL, 10);
        if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
            throw yy::parser::syntax_error(loc, "integer is out of range: " + s);
        return yy::parser::make_INT((int)n, loc);
    } else {
        double n = strtod(s.c_str(), NULL);
        return yy::parser::make_FLOAT(n, loc);
    }
}

yy::parser::symbol_type make_IDENTIFIER(const std::string& s, const yy::parser::location_type& loc) {
    std::map<std::string, std::function<yy::parser::symbol_type(const yy::parser::location_type&)>> keywords = {
            {"if", yy::parser::make_IFTRUE },
            {"iftrue", yy::parser::make_IFTRUE },
            {"iffalse", yy::parser::make_IFFALSE },
            {"goto", yy::parser::make_GOTO },
            {"halt", yy::parser::make_HALT },
            {"getparam", yy::parser::make_GETPARAM },
            {"putparam", yy::parser::make_PUTPARAM },
            {"call", yy::parser::make_CALL },
            {"nop", yy::parser::make_NOP },
            {"return", yy::parser::make_RETURN },
            {"print_to_console", yy::parser::make_PRINT },
            {"block", yy::parser::make_BLOCK },
    };

    if (auto func = keywords.find(s); func != keywords.end()) return func->second(loc);
    else return yy::parser::make_IDENTIFIER(s, loc);
}

void ParseDriver::scan_begin()
{
    yy_flex_debug = trace_scanning;
    // yy_flex_debug = 1;
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