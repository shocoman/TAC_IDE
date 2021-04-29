%{
    //#pragma warning (disable : 4005)
    #include <climits>
    #include <cstring>
    #include <string>
    #include <algorithm>
    #include <functional>
    #include <map>
    #include "../driver/driver.hpp"
%}

%option noyywrap nounput noinput batch debug

%{
    yy::Parser::symbol_type make_NUMBER(const std::string& s, const yy::Parser::location_type& loc, bool);
    yy::Parser::symbol_type make_IDENTIFIER(const std::string& s, const yy::Parser::location_type& loc);

    #define YY_USER_ACTION loc.columns(yyleng); // update location
%}


id  [a-zA-Z$_][a-zA-Z_0-9]*
int [0-9]+
float [0-9]+\.[0-9]+?
string \"([^\\\"]|\\.)*\"
char '([^'\\\n]|\\.)'
bool true|false

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
\n                          loc.lines(yyleng); loc.step(); return yy::Parser::make_NEWLINE(loc);
{blank}+                    loc.step();
{line_comment_with_nl}      loc.lines(yytext[yyleng-1] == '\n' ? 1 : 0); loc.step();
{line_comment}              loc.lines(yytext[yyleng-1] == '\n' ? 1 : 0); loc.step();

"="     return yy::Parser::make_ASSIGN(loc);
"+"     return yy::Parser::make_PLUS(loc);
"-"     return yy::Parser::make_MINUS(loc);
"*"     return yy::Parser::make_MULT(loc);
"/"     return yy::Parser::make_DIV(loc);
"&"     return yy::Parser::make_REF(loc);
"&&"     return yy::Parser::make_AND(loc);
"||"     return yy::Parser::make_OR(loc);
"!"     return yy::Parser::make_NOT(loc);
"^"     return yy::Parser::make_XOR(loc);
"%"     return yy::Parser::make_MODULUS(loc);

"<"     return yy::Parser::make_CMP_LT(loc);
"<="    return yy::Parser::make_CMP_LTE(loc);
">"     return yy::Parser::make_CMP_GT(loc);
">="    return yy::Parser::make_CMP_GTE(loc);
"=="    return yy::Parser::make_CMP_EQ(loc);
"!="    return yy::Parser::make_CMP_NEQ(loc);


"."     return yy::Parser::make_DOT(loc);
","     return yy::Parser::make_COMMA(loc);
";"     return yy::Parser::make_SEMI(loc);
":"     return yy::Parser::make_COLON(loc);
"("     return yy::Parser::make_LPAREN(loc);
")"     return yy::Parser::make_RPAREN(loc);
"["     return yy::Parser::make_LBRACKET(loc);
"]"     return yy::Parser::make_RBRACKET(loc);


{char}   return yy::Parser::make_CHAR(yytext[1], loc);
{bool}   return yy::Parser::make_BOOL(yytext, loc);
{string} return yy::Parser::make_STRING(yytext, loc);
{float}  return make_NUMBER(yytext, loc, false);
{int}    return make_NUMBER(yytext, loc, true);
{id}     return make_IDENTIFIER(yytext, loc);

.       {
            throw yy::Parser::syntax_error(loc, "invalid character: " + std::string(yytext));
        }


<<EOF>> return yy::Parser::make_EOF(loc);
%%

yy::Parser::symbol_type make_NUMBER(const std::string& s, const yy::Parser::location_type& loc, bool is_int) {
    errno = 0;
    if (is_int) {
        long n = strtol(s.c_str(), NULL, 10);
        if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE))
            throw yy::Parser::syntax_error(loc, "integer is out of range: " + s);
        return yy::Parser::make_INT((int)n, loc);
    } else {
        double n = strtod(s.c_str(), NULL);
        return yy::Parser::make_FLOAT(n, loc);
    }
}

yy::Parser::symbol_type make_IDENTIFIER(const std::string &s, const yy::Parser::location_type& loc) {
    std::map<std::string, std::function<yy::Parser::symbol_type(const yy::Parser::location_type&)>> keywords = {
            {"if", yy::Parser::make_IFTRUE },
            {"iftrue", yy::Parser::make_IFTRUE },
            {"iffalse", yy::Parser::make_IFFALSE },
            {"goto", yy::Parser::make_GOTO },
            {"halt", yy::Parser::make_HALT },
            {"getparam", yy::Parser::make_GETPARAM },
            {"putparam", yy::Parser::make_PUTPARAM },
            {"call", yy::Parser::make_CALL },
            {"nop", yy::Parser::make_NOP },
            {"return", yy::Parser::make_RETURN },
            {"print", yy::Parser::make_PRINT },
            {"block", yy::Parser::make_BLOCK },
    };
    // convert string to lower case
    auto lowercase_s = s;
    std::transform(s.begin(), s.end(), lowercase_s.begin(), [](unsigned char c){ return std::tolower(c); });
    if (auto func = keywords.find(lowercase_s); func != keywords.end()) return func->second(loc);
    else return yy::Parser::make_IDENTIFIER(s, loc);
}

void ParseDriver::scan_from_file_begin() {
    yylex_destroy();
    yy_flex_debug = trace_scanning;
    if (file_name.empty() || file_name == "-")
        yyin = stdin;
    else if (!(yyin = fopen(file_name.c_str(), "r"))) {
        std::cerr << "cannot open " << file_name << ": " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ParseDriver::scan_from_string_begin(const std::string &str) {
    yylex_destroy();
    yy_flex_debug = trace_scanning;
    yy_scan_string(str.c_str());
}

void ParseDriver::scan_end() {
    fclose(yyin);
}
