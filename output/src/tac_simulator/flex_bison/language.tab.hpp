/* A Bison parser, made by GNU Bison 3.7.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_MEDIA_VICTOR_264C09FD4C09C90F1_PROGRAMMING_C_TAC_PARSER_OUTPUT_SRC_TAC_SIMULATOR_FLEX_BISON_LANGUAGE_TAB_HPP_INCLUDED
# define YY_YY_MEDIA_VICTOR_264C09FD4C09C90F1_PROGRAMMING_C_TAC_PARSER_OUTPUT_SRC_TAC_SIMULATOR_FLEX_BISON_LANGUAGE_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    TOKENS_START = 258,            /* TOKENS_START  */
    _ID = 259,                     /* _ID  */
    _IDTEMP = 260,                 /* _IDTEMP  */
    _DCONST = 261,                 /* _DCONST  */
    _DCONSTE = 262,                /* _DCONSTE  */
    _ICONST = 263,                 /* _ICONST  */
    _CCONST = 264,                 /* _CCONST  */
    _SCONST = 265,                 /* _SCONST  */
    _ARRAYTYPE = 266,              /* _ARRAYTYPE  */
    _BYTETYPE = 267,               /* _BYTETYPE  */
    _WORDTYPE = 268,               /* _WORDTYPE  */
    _LONGTYPE = 269,               /* _LONGTYPE  */
    _DOUBLETYPE = 270,             /* _DOUBLETYPE  */
    _ASCIITYPE = 271,              /* _ASCIITYPE  */
    _BYTEARRAYTYPE = 272,          /* _BYTEARRAYTYPE  */
    _WORDARRAYTYPE = 273,          /* _WORDARRAYTYPE  */
    _LONGARRAYTYPE = 274,          /* _LONGARRAYTYPE  */
    _DOUBLEARRAYTYPE = 275,        /* _DOUBLEARRAYTYPE  */
    _ASCIIARRAYTYPE = 276,         /* _ASCIIARRAYTYPE  */
    _TOASCII = 277,                /* _TOASCII  */
    _TOBYTE = 278,                 /* _TOBYTE  */
    _TOWORD = 279,                 /* _TOWORD  */
    _TOLONG = 280,                 /* _TOLONG  */
    _TODOUBLE = 281,               /* _TODOUBLE  */
    _IFTRUE = 282,                 /* _IFTRUE  */
    _IFFALSE = 283,                /* _IFFALSE  */
    _GOTO = 284,                   /* _GOTO  */
    _PUTPARAM = 285,               /* _PUTPARAM  */
    _GETPARAM = 286,               /* _GETPARAM  */
    _CALL = 287,                   /* _CALL  */
    _RETURN = 288,                 /* _RETURN  */
    _IWRITE = 289,                 /* _IWRITE  */
    _FWRITE = 290,                 /* _FWRITE  */
    _CWRITE = 291,                 /* _CWRITE  */
    _SWRITE = 292,                 /* _SWRITE  */
    _IREAD = 293,                  /* _IREAD  */
    _FREAD = 294,                  /* _FREAD  */
    _CREAD = 295,                  /* _CREAD  */
    _SREAD = 296,                  /* _SREAD  */
    _SEPARATOR = 297,              /* _SEPARATOR  */
    TOKENS_END = 298,              /* TOKENS_END  */
    _EQ = 299,                     /* _EQ  */
    _NEQ = 300,                    /* _NEQ  */
    _GEQ = 301,                    /* _GEQ  */
    _LEQ = 302,                    /* _LEQ  */
    _AND = 303,                    /* _AND  */
    _OR = 304,                     /* _OR  */
    _IS = 305,                     /* _IS  */
    UMINUS = 306,                  /* UMINUS  */
    UOPNOT = 307                   /* UOPNOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_MEDIA_VICTOR_264C09FD4C09C90F1_PROGRAMMING_C_TAC_PARSER_OUTPUT_SRC_TAC_SIMULATOR_FLEX_BISON_LANGUAGE_TAB_HPP_INCLUDED  */
