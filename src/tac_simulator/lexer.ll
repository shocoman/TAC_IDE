%{
#if defined _WIN32
    #include <io.h> // Для isatty
#elif defined _WIN64
    #include <io.h> // Для isatty
#endif
#ifdef MSVC
    #define isatty _isatty // В VC isatty назван _isatty
#endif

#include "language.tab.hpp"

%}

%option noyywrap nodefault yylineno

stringch [^"\n]
char [^']
floatE [0-9]+(\.|,)[0-9]+("e"|"E")(\+|-)[0-9]+

%%
[ \t] { }

"//".*$ { yylineno -= 1; }


{floatE} {return _DCONSTE;}
0|[1-9][0-9]* { return _ICONST;}
[0-9]+(\.|,)[0-9]+ { return _DCONST;}
"'"{char}"'" { return _CCONST;}

"if"|"iftrue" { return _IFTRUE;}
"iffalse" { return _IFFALSE;}
"goto" { return _GOTO;}
"putparam" { return _PUTPARAM;}
"getparam" { return _GETPARAM;}
"call" { return _CALL;}
"return" { return _RETURN;}
"iwrite" { return _IWRITE;}
"fwrite" { return _FWRITE;}
"cwrite" { return _CWRITE;}
"swrite" { return _SWRITE;}
"iread" { return _IREAD;}
"fread" { return _FREAD;}
"cread" { return _CREAD;}
"sread" { return _SREAD;}

"toascii" { return _TOASCII;}
"tobyte" { return _TOBYTE;}
"toword" { return _TOWORD;}
"tolong" { return _TOLONG;}
"todouble" { return _TODOUBLE;}


".block" { return _ARRAYTYPE;}
".byte" { return _BYTETYPE;}
".word" { return _WORDTYPE;}
".long" { return _LONGTYPE;}
".double" { return _DOUBLETYPE;}
".ascii" { return _ASCIITYPE;}


"byte" { return _BYTEARRAYTYPE;}
"word" { return _WORDARRAYTYPE;}
"long" { return _LONGARRAYTYPE;}
"double" { return _DOUBLEARRAYTYPE;}
"ascii" { return _ASCIIARRAYTYPE;}

 /* операции сравнения */
">=" { return _GEQ;}
"<=" { return _LEQ;}
">"  { return yytext[0];}
"<"  { return yytext[0];}
"!=" { return _NEQ;}
"==" { return _EQ;}


 /* однознаковые операции */
"+" { return yytext[0];}
"-" { return yytext[0];}
"*" { return yytext[0];}
"/" { return yytext[0];}
"%" { return yytext[0];}

"=" { return _IS;}

"," { return yytext[0];}
"(" { return yytext[0];}
")" { return yytext[0];}
"{" { return yytext[0];}
"[" { return yytext[0];}
"]" { return yytext[0];}
"}" { return yytext[0];}
":" { return yytext[0];}


 /* операции логические */
"&&" { return _AND;}
"||" { return _OR;}

"^"  { return yytext[0];}
"!"  { return yytext[0];}

"$t"[0-9]+ { return _IDTEMP;}
[A-Za-z][A-Za-z0-9_]* { return _ID;}

\"{stringch}*$ {printf("Error at line %d: Unfinished string literal - %s.\n", yylineno, &yytext[1]);}
\"{stringch}*\" {char string[256] = ""; int len = strlen(yytext)-2; strncpy(string, &yytext[1], len);  return _SCONST;}

[\n] {return _SEPARATOR;}

. { printf("Lexer Error at line %d: '%s' - not in alphabet.\n", yylineno, yytext); }

%%

