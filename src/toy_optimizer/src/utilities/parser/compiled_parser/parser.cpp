// A Bison parser, made by GNU Bison 3.7.1.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "parser.hpp"


// Unqualified %code blocks.
#line 17 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"

    #include "../parser/driver/driver.hpp"

#line 50 "../src/utilities/parser/compiled_parser/parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 142 "../src/utilities/parser/compiled_parser/parser.cpp"

  /// Build a parser object.
   Parser :: Parser  (ParseDriver& drv_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      yy_lac_established_ (false),
      drv (drv_yyarg)
  {}

   Parser ::~ Parser  ()
  {}

   Parser ::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | symbol kinds.  |
  `---------------*/



  // by_state.
   Parser ::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

   Parser ::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
   Parser ::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
   Parser ::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

   Parser ::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

   Parser ::symbol_kind_type
   Parser ::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

   Parser ::stack_symbol_type::stack_symbol_type ()
  {}

   Parser ::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_dest: // dest
        value.YY_MOVE_OR_COPY< Dest > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_term: // term
        value.YY_MOVE_OR_COPY< Operand > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_value: // value
      case symbol_kind::S_quadruple: // quadruple
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_goto: // goto
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_var_declaration: // var_declaration
        value.YY_MOVE_OR_COPY< Quad > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_CHAR: // "char"
        value.YY_MOVE_OR_COPY< char > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FLOAT: // "float"
        value.YY_MOVE_OR_COPY< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INT: // "int"
        value.YY_MOVE_OR_COPY< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_BOOL: // "bool"
      case symbol_kind::S_label: // label
        value.YY_MOVE_OR_COPY< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

   Parser ::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.location))
  {
    switch (that.kind ())
    {
      case symbol_kind::S_dest: // dest
        value.move< Dest > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_term: // term
        value.move< Operand > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_value: // value
      case symbol_kind::S_quadruple: // quadruple
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_goto: // goto
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_var_declaration: // var_declaration
        value.move< Quad > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_CHAR: // "char"
        value.move< char > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_FLOAT: // "float"
        value.move< double > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_INT: // "int"
        value.move< int > (YY_MOVE (that.value));
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_BOOL: // "bool"
      case symbol_kind::S_label: // label
        value.move< std::string > (YY_MOVE (that.value));
        break;

      default:
        break;
    }

    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
   Parser ::stack_symbol_type&
   Parser ::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_dest: // dest
        value.copy< Dest > (that.value);
        break;

      case symbol_kind::S_term: // term
        value.copy< Operand > (that.value);
        break;

      case symbol_kind::S_value: // value
      case symbol_kind::S_quadruple: // quadruple
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_goto: // goto
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_var_declaration: // var_declaration
        value.copy< Quad > (that.value);
        break;

      case symbol_kind::S_CHAR: // "char"
        value.copy< char > (that.value);
        break;

      case symbol_kind::S_FLOAT: // "float"
        value.copy< double > (that.value);
        break;

      case symbol_kind::S_INT: // "int"
        value.copy< int > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_BOOL: // "bool"
      case symbol_kind::S_label: // label
        value.copy< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    return *this;
  }

   Parser ::stack_symbol_type&
   Parser ::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    switch (that.kind ())
    {
      case symbol_kind::S_dest: // dest
        value.move< Dest > (that.value);
        break;

      case symbol_kind::S_term: // term
        value.move< Operand > (that.value);
        break;

      case symbol_kind::S_value: // value
      case symbol_kind::S_quadruple: // quadruple
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_goto: // goto
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_var_declaration: // var_declaration
        value.move< Quad > (that.value);
        break;

      case symbol_kind::S_CHAR: // "char"
        value.move< char > (that.value);
        break;

      case symbol_kind::S_FLOAT: // "float"
        value.move< double > (that.value);
        break;

      case symbol_kind::S_INT: // "int"
        value.move< int > (that.value);
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_BOOL: // "bool"
      case symbol_kind::S_label: // label
        value.move< std::string > (that.value);
        break;

      default:
        break;
    }

    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
   Parser ::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);
  }

#if YYDEBUG
  template <typename Base>
  void
   Parser ::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YYUSE (yykind);
        yyo << ')';
      }
  }
#endif

  void
   Parser ::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
   Parser ::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
   Parser ::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
   Parser ::debug_stream () const
  {
    return *yycdebug_;
  }

  void
   Parser ::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


   Parser ::debug_level_type
   Parser ::debug_level () const
  {
    return yydebug_;
  }

  void
   Parser ::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

   Parser ::state_type
   Parser ::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
   Parser ::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
   Parser ::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
   Parser ::operator() ()
  {
    return parse ();
  }

  int
   Parser ::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    /// Discard the LAC context in case there still is one left from a
    /// previous invocation.
    yy_lac_discard_ ("init");

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            symbol_type yylookahead (yylex (drv));
            yyla.move (yylookahead);
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        if (!yy_lac_establish_ (yyla.kind ()))
           goto yyerrlab;
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        if (!yy_lac_establish_ (yyla.kind ()))
           goto yyerrlab;

        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    yy_lac_discard_ ("shift");
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* Variants are always initialized to an empty instance of the
         correct type. The default '$$ = $1' action is NOT applied
         when using variants.  */
      switch (yyr1_[yyn])
    {
      case symbol_kind::S_dest: // dest
        yylhs.value.emplace< Dest > ();
        break;

      case symbol_kind::S_term: // term
        yylhs.value.emplace< Operand > ();
        break;

      case symbol_kind::S_value: // value
      case symbol_kind::S_quadruple: // quadruple
      case symbol_kind::S_if_statement: // if_statement
      case symbol_kind::S_goto: // goto
      case symbol_kind::S_assignment: // assignment
      case symbol_kind::S_array_assignment: // array_assignment
      case symbol_kind::S_var_declaration: // var_declaration
        yylhs.value.emplace< Quad > ();
        break;

      case symbol_kind::S_CHAR: // "char"
        yylhs.value.emplace< char > ();
        break;

      case symbol_kind::S_FLOAT: // "float"
        yylhs.value.emplace< double > ();
        break;

      case symbol_kind::S_INT: // "int"
        yylhs.value.emplace< int > ();
        break;

      case symbol_kind::S_IDENTIFIER: // "identifier"
      case symbol_kind::S_STRING: // "string"
      case symbol_kind::S_BOOL: // "bool"
      case symbol_kind::S_label: // label
        yylhs.value.emplace< std::string > ();
        break;

      default:
        break;
    }


      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 4: // statement: label mb_newline
#line 98 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                           { drv.labels.emplace(yystack_[1].value.as < std::string > (), drv.quadruples.size()); }
#line 732 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 5: // statement: quadruple newlines
#line 99 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                           { drv.quadruples.push_back(yystack_[1].value.as < Quad > ()); }
#line 738 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 11: // quadruple: assignment
#line 105 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
    { yylhs.value.as < Quad > () = yystack_[0].value.as < Quad > (); }
#line 744 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 12: // quadruple: if_statement
#line 106 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
    { yylhs.value.as < Quad > () = yystack_[0].value.as < Quad > (); }
#line 750 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 13: // quadruple: goto
#line 107 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
    { yylhs.value.as < Quad > () = yystack_[0].value.as < Quad > (); }
#line 756 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 14: // quadruple: "halt"
#line 108 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                        { yylhs.value.as < Quad > () = Quad({}, {}, Quad::Type::Halt); }
#line 762 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 15: // quadruple: "nop"
#line 109 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                        { yylhs.value.as < Quad > () = Quad({}, {}, Quad::Type::Nop); }
#line 768 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 16: // quadruple: "return" term
#line 110 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                        { yylhs.value.as < Quad > () = Quad(yystack_[0].value.as < Operand > (), {}, Quad::Type::Return); }
#line 774 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 17: // quadruple: "print_to_console" term
#line 111 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                        { yylhs.value.as < Quad > () = Quad(yystack_[0].value.as < Operand > (), {}, Quad::Type::Print); }
#line 780 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 18: // quadruple: "call" "identifier" "," "int"
#line 112 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                        {
         yylhs.value.as < Quad > () = Quad(Operand(yystack_[2].value.as < std::string > (), Operand::Type::None), Operand(std::to_string(yystack_[0].value.as < int > ()), Operand::Type::None), Quad::Type::Call);
       }
#line 788 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 19: // quadruple: var_declaration
#line 115 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
    { yylhs.value.as < Quad > () = yystack_[0].value.as < Quad > (); }
#line 794 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 20: // quadruple: "putparam" term
#line 116 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                        { yylhs.value.as < Quad > () = Quad(yystack_[0].value.as < Operand > (), {}, Quad::Type::Putparam); }
#line 800 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 21: // quadruple: "getparam" "identifier"
#line 117 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                        { yylhs.value.as < Quad > () = Quad(yystack_[0].value.as < std::string > (), {}, Quad::Type::Getparam); }
#line 806 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 22: // var_declaration: label "." "identifier" term
#line 122 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                              {
                                  yylhs.value.as < Quad > () = Quad(Operand(yystack_[1].value.as < std::string > (), Operand::Type::None), yystack_[0].value.as < Operand > (), Quad::Type::VarDeclaration);
                                  yylhs.value.as < Quad > ().dest = Dest(yystack_[3].value.as < std::string > (), Dest::Type::Var);
                              }
#line 815 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 23: // var_declaration: label "." "block" "int" "," "identifier" "," term
#line 127 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                              {
                                  yylhs.value.as < Quad > () = Quad(Operand(yystack_[2].value.as < std::string > (), Operand::Type::None), yystack_[0].value.as < Operand > (), Quad::Type::ArrayDeclaration);
                                  yylhs.value.as < Quad > ().ops.push_back(Operand(std::to_string(yystack_[4].value.as < int > ()), Operand::Type::None));
                                  yylhs.value.as < Quad > ().dest = Dest(yystack_[7].value.as < std::string > (), Dest::Type::Var);
                              }
#line 825 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 24: // assignment: dest "=" value
#line 134 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                   { yystack_[0].value.as < Quad > ().dest = yystack_[2].value.as < Dest > (); yylhs.value.as < Quad > () = yystack_[0].value.as < Quad > (); }
#line 831 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 25: // assignment: array_assignment
#line 135 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
    { yylhs.value.as < Quad > () = yystack_[0].value.as < Quad > (); }
#line 837 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 26: // array_assignment: "identifier" "[" term "]" "=" term
#line 138 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                                               { yylhs.value.as < Quad > () = Quad(yystack_[3].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::ArraySet);
                                                                   yylhs.value.as < Quad > ().dest = Dest(yystack_[5].value.as < std::string > (), Dest::Type::Var); }
#line 844 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 27: // if_statement: "if" term "goto" "identifier"
#line 143 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                             { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), {}, Quad::Type::IfTrue);
                                                yylhs.value.as < Quad > ().dest = Dest(yystack_[0].value.as < std::string > (), Dest::Type::JumpLabel); }
#line 851 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 28: // if_statement: "iffalse" term "goto" "identifier"
#line 145 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                             { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), {}, Quad::Type::IfFalse);
                                                yylhs.value.as < Quad > ().dest = Dest(yystack_[0].value.as < std::string > (), Dest::Type::JumpLabel);}
#line 858 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 29: // goto: "goto" "identifier"
#line 149 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                { yylhs.value.as < Quad > () = Quad({}, {}, Quad::Type::Goto);
                                yylhs.value.as < Quad > ().dest = Dest(yystack_[0].value.as < std::string > (), Dest::Type::JumpLabel);}
#line 865 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 30: // dest: "identifier"
#line 153 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                { yylhs.value.as < Dest > () = Dest(yystack_[0].value.as < std::string > (), Dest::Type::Var); }
#line 871 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 31: // dest: "*" "identifier"
#line 154 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                { yylhs.value.as < Dest > () = Dest(yystack_[0].value.as < std::string > (), Dest::Type::Deref); }
#line 877 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 32: // label: "identifier" ":"
#line 158 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                     { yylhs.value.as < std::string > () = yystack_[1].value.as < std::string > (); }
#line 883 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 33: // value: term
#line 162 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[0].value.as < Operand > (), {}, Quad::Type::Assign); }
#line 889 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 34: // value: "*" term
#line 163 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[0].value.as < Operand > (), {}, Quad::Type::Deref); }
#line 895 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 35: // value: "-" term
#line 164 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[0].value.as < Operand > (), {}, Quad::Type::UMinus); }
#line 901 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 36: // value: "&" term
#line 165 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[0].value.as < Operand > (), {}, Quad::Type::Ref); }
#line 907 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 37: // value: term "[" term "]"
#line 166 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[3].value.as < Operand > (), yystack_[1].value.as < Operand > (), Quad::Type::ArrayGet); }
#line 913 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 38: // value: term "+" term
#line 167 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Add); }
#line 919 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 39: // value: term "-" term
#line 168 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Sub); }
#line 925 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 40: // value: term "*" term
#line 169 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Mult); }
#line 931 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 41: // value: term "&&" term
#line 170 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::And); }
#line 937 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 42: // value: term "||" term
#line 171 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Or); }
#line 943 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 43: // value: term "/" term
#line 172 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Div); }
#line 949 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 44: // value: term "<" term
#line 173 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Lt); }
#line 955 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 45: // value: term "<=" term
#line 174 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Lte); }
#line 961 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 46: // value: term ">" term
#line 175 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Gt); }
#line 967 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 47: // value: term ">=" term
#line 176 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Gte); }
#line 973 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 48: // value: term "==" term
#line 177 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Eq); }
#line 979 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 49: // value: term "!=" term
#line 178 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                            { yylhs.value.as < Quad > () = Quad(yystack_[2].value.as < Operand > (), yystack_[0].value.as < Operand > (), Quad::Type::Neq); }
#line 985 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 50: // value: "call" "identifier" "," "int"
#line 179 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                                        {
        yylhs.value.as < Quad > () = Quad(Operand(yystack_[2].value.as < std::string > (), Operand::Type::None), Operand(std::to_string(yystack_[0].value.as < int > ()), Operand::Type::None), Quad::Type::Call);
       }
#line 993 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 51: // term: "identifier"
#line 185 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                    { yylhs.value.as < Operand > () = Operand(yystack_[0].value.as < std::string > (), Operand::Type::Var); }
#line 999 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 52: // term: "string"
#line 186 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                    { yylhs.value.as < Operand > () = Operand(yystack_[0].value.as < std::string > (), Operand::Type::LString); }
#line 1005 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 53: // term: "char"
#line 187 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                    { yylhs.value.as < Operand > () = Operand(std::string(1, yystack_[0].value.as < char > ()), Operand::Type::LChar); }
#line 1011 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 54: // term: "int"
#line 188 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                    { yylhs.value.as < Operand > () = Operand(std::to_string(yystack_[0].value.as < int > ()), Operand::Type::LInt); }
#line 1017 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 55: // term: "float"
#line 189 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                    { yylhs.value.as < Operand > () = Operand(std::to_string(yystack_[0].value.as < double > ()), Operand::Type::LDouble); }
#line 1023 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;

  case 56: // term: "bool"
#line 190 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"
                    { yylhs.value.as < Operand > () = Operand(yystack_[0].value.as < std::string > (), Operand::Type::LBool); }
#line 1029 "../src/utilities/parser/compiled_parser/parser.cpp"
    break;


#line 1033 "../src/utilities/parser/compiled_parser/parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      yy_lac_discard_ ("error recovery");
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
   Parser ::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
   Parser ::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr;
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              else
                goto append;

            append:
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

  std::string
   Parser ::symbol_name (symbol_kind_type yysymbol)
  {
    return yytnamerr_ (yytname_[yysymbol]);
  }



  //  Parser ::context.
   Parser ::context::context (const  Parser & yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
   Parser ::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

#if YYDEBUG
    // Execute LAC once. We don't care if it is successful, we
    // only do it for the sake of debugging output.
    if (!yyparser_.yy_lac_established_)
      yyparser_.yy_lac_check_ (yyla_.kind ());
#endif

    for (int yyx = 0; yyx < YYNTOKENS; ++yyx)
      {
        symbol_kind_type yysym = YY_CAST (symbol_kind_type, yyx);
        if (yysym != symbol_kind::S_YYerror
            && yysym != symbol_kind::S_YYUNDEF
            && yyparser_.yy_lac_check_ (yysym))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = yysym;
          }
      }
    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }


  bool
   Parser ::yy_lac_check_ (symbol_kind_type yytoken) const
  {
    // Logically, the yylac_stack's lifetime is confined to this function.
    // Clear it, to get rid of potential left-overs from previous call.
    yylac_stack_.clear ();
    // Reduce until we encounter a shift and thereby accept the token.
#if YYDEBUG
    YYCDEBUG << "LAC: checking lookahead " << symbol_name (yytoken) << ':';
#endif
    std::ptrdiff_t lac_top = 0;
    while (true)
      {
        state_type top_state = (yylac_stack_.empty ()
                                ? yystack_[lac_top].state
                                : yylac_stack_.back ());
        int yyrule = yypact_[+top_state];
        if (yy_pact_value_is_default_ (yyrule)
            || (yyrule += yytoken) < 0 || yylast_ < yyrule
            || yycheck_[yyrule] != yytoken)
          {
            // Use the default action.
            yyrule = yydefact_[+top_state];
            if (yyrule == 0)
              {
                YYCDEBUG << " Err\n";
                return false;
              }
          }
        else
          {
            // Use the action from yytable.
            yyrule = yytable_[yyrule];
            if (yy_table_value_is_error_ (yyrule))
              {
                YYCDEBUG << " Err\n";
                return false;
              }
            if (0 < yyrule)
              {
                YYCDEBUG << " S" << yyrule << '\n';
                return true;
              }
            yyrule = -yyrule;
          }
        // By now we know we have to simulate a reduce.
        YYCDEBUG << " R" << yyrule - 1;
        // Pop the corresponding number of values from the stack.
        {
          std::ptrdiff_t yylen = yyr2_[yyrule];
          // First pop from the LAC stack as many tokens as possible.
          std::ptrdiff_t lac_size = std::ptrdiff_t (yylac_stack_.size ());
          if (yylen < lac_size)
            {
              yylac_stack_.resize (std::size_t (lac_size - yylen));
              yylen = 0;
            }
          else if (lac_size)
            {
              yylac_stack_.clear ();
              yylen -= lac_size;
            }
          // Only afterwards look at the main stack.
          // We simulate popping elements by incrementing lac_top.
          lac_top += yylen;
        }
        // Keep top_state in sync with the updated stack.
        top_state = (yylac_stack_.empty ()
                     ? yystack_[lac_top].state
                     : yylac_stack_.back ());
        // Push the resulting state of the reduction.
        state_type state = yy_lr_goto_state_ (top_state, yyr1_[yyrule]);
        YYCDEBUG << " G" << int (state);
        yylac_stack_.push_back (state);
      }
  }

  // Establish the initial context if no initial context currently exists.
  bool
   Parser ::yy_lac_establish_ (symbol_kind_type yytoken)
  {
    /* Establish the initial context for the current lookahead if no initial
       context is currently established.

       We define a context as a snapshot of the parser stacks.  We define
       the initial context for a lookahead as the context in which the
       parser initially examines that lookahead in order to select a
       syntactic action.  Thus, if the lookahead eventually proves
       syntactically unacceptable (possibly in a later context reached via a
       series of reductions), the initial context can be used to determine
       the exact set of tokens that would be syntactically acceptable in the
       lookahead's place.  Moreover, it is the context after which any
       further semantic actions would be erroneous because they would be
       determined by a syntactically unacceptable token.

       yy_lac_establish_ should be invoked when a reduction is about to be
       performed in an inconsistent state (which, for the purposes of LAC,
       includes consistent states that don't know they're consistent because
       their default reductions have been disabled).

       For parse.lac=full, the implementation of yy_lac_establish_ is as
       follows.  If no initial context is currently established for the
       current lookahead, then check if that lookahead can eventually be
       shifted if syntactic actions continue from the current context.  */
    if (!yy_lac_established_)
      {
#if YYDEBUG
        YYCDEBUG << "LAC: initial context established for "
                 << symbol_name (yytoken) << '\n';
#endif
        yy_lac_established_ = true;
        return yy_lac_check_ (yytoken);
      }
    return true;
  }

  // Discard any previous initial lookahead context.
  void
   Parser ::yy_lac_discard_ (const char* evt)
  {
   /* Discard any previous initial lookahead context because of Event,
      which may be a lookahead change or an invalidation of the currently
      established initial context for the current lookahead.

      The most common example of a lookahead change is a shift.  An example
      of both cases is syntax error recovery.  That is, a syntax error
      occurs when the lookahead is syntactically erroneous for the
      currently established initial context, so error recovery manipulates
      the parser stacks to try to find a new initial context in which the
      current lookahead is syntactically acceptable.  If it fails to find
      such a context, it discards the lookahead.  */
    if (yy_lac_established_)
      {
        YYCDEBUG << "LAC: initial context discarded due to "
                 << evt << '\n';
        yy_lac_established_ = false;
      }
  }

  int
   Parser ::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
         In the first two cases, it might appear that the current syntax
         error should have been detected in the previous state when
         yy_lac_check was invoked.  However, at that time, there might
         have been a different syntax error that discarded a different
         initial context during error recovery, leaving behind the
         current lookahead.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
   Parser ::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char  Parser ::yypact_ninf_ = -32;

  const signed char  Parser ::yytable_ninf_ = -1;

  const signed char
   Parser ::yypact_[] =
  {
     -32,     9,   -32,    64,    64,   -31,   -32,   -26,    64,   -15,
     -32,    64,   -14,   -29,    64,    10,     8,   -32,   -32,   -32,
     -32,   -32,   -12,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
      24,    25,   -32,    -3,   -32,   -32,   -32,   -32,    64,   -32,
     -32,    21,   -32,   -32,    18,   -32,   -11,   -32,    -2,    -1,
      -8,    11,     3,    64,    64,    64,   -32,    65,   -32,     1,
      64,   -32,   -32,   -32,    29,    12,   -32,   -32,   -32,    64,
      64,    64,    64,    64,    64,    64,    64,    64,    64,    64,
      64,    64,    13,   -32,    64,     4,   -32,   -32,   -32,   -32,
     -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,    22,    16,
     -32,   -32,   -32,    20,    64,   -32
  };

  const signed char
   Parser ::yydefact_[] =
  {
       2,     0,     1,     0,     0,     0,    14,     0,     0,     0,
      15,     0,     0,    30,     0,     0,     0,    12,    13,    11,
      25,    19,     6,     3,    51,    52,    56,    53,    54,    55,
       0,     0,    29,     0,    20,    21,    16,    31,     0,    32,
      17,     0,     8,     9,     5,     7,     0,     4,     0,     0,
       0,     0,     0,     0,     0,     0,    24,    33,    10,     0,
       0,    27,    28,    18,     0,     0,    35,    34,    36,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    22,     0,     0,    38,    39,    40,    43,
      41,    42,    44,    45,    46,    47,    48,    49,     0,     0,
      26,    50,    37,     0,     0,    23
  };

  const signed char
   Parser ::yypgoto_[] =
  {
     -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,   -32,
      -4,   -32,   -32,   -32,   -32
  };

  const signed char
   Parser ::yydefgoto_[] =
  {
      -1,    15,    56,    16,    17,    18,    19,    20,    21,    22,
      30,     1,    23,    47,    44
  };

  const signed char
   Parser ::yytable_[] =
  {
      31,    45,    38,    59,    34,    39,    32,    36,    42,     2,
      40,    33,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    43,    35,    37,    46,    41,    60,    12,    52,    48,
      49,    58,    50,    63,    51,    61,    62,    57,    53,    54,
      65,    55,    82,    64,    84,   101,    13,    85,    99,    66,
      67,    68,    14,   103,   102,   104,    83,     0,    24,    25,
      26,    27,    28,    29,     0,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,     0,     0,
     100,    69,    70,    71,    72,     0,    73,    74,    75,    76,
      77,    78,    79,    80,     0,     0,    81,     0,     0,     0,
     105,    24,    25,    26,    27,    28,    29
  };

  const signed char
   Parser ::yycheck_[] =
  {
       4,    13,    31,    14,     8,    34,    37,    11,     0,     0,
      14,    37,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    13,    37,    37,    36,    15,    37,    18,     7,     5,
       5,    13,    35,    41,    38,    37,    37,    41,    17,    18,
      37,    20,    41,    32,    15,    41,    37,    35,    35,    53,
      54,    55,    43,    37,    32,    35,    60,    -1,    37,    38,
      39,    40,    41,    42,    -1,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    -1,    -1,
      84,    16,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    -1,    31,    -1,    -1,    -1,
     104,    37,    38,    39,    40,    41,    42
  };

  const signed char
   Parser ::yystos_[] =
  {
       0,    55,     0,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    18,    37,    43,    45,    47,    48,    49,    50,
      51,    52,    53,    56,    37,    38,    39,    40,    41,    42,
      54,    54,    37,    37,    54,    37,    54,    37,    31,    34,
      54,    15,     0,    13,    58,    13,    36,    57,     5,     5,
      35,    54,     7,    17,    18,    20,    46,    54,    13,    14,
      37,    37,    37,    41,    32,    37,    54,    54,    54,    16,
      17,    18,    19,    21,    22,    23,    24,    25,    26,    27,
      28,    31,    41,    54,    15,    35,    54,    54,    54,    54,
      54,    54,    54,    54,    54,    54,    54,    54,    54,    35,
      54,    41,    32,    37,    35,    54
  };

  const signed char
   Parser ::yyr1_[] =
  {
       0,    44,    55,    55,    56,    56,    57,    57,    58,    58,
      58,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    52,    52,    50,    50,    51,    48,    48,    49,
      45,    45,    53,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    54,    54,    54,    54,    54,    54
  };

  const signed char
   Parser ::yyr2_[] =
  {
       0,     2,     0,     2,     2,     2,     0,     1,     1,     1,
       2,     1,     1,     1,     1,     1,     2,     2,     4,     1,
       2,     2,     4,     8,     3,     1,     6,     4,     4,     2,
       1,     2,     2,     1,     2,     2,     2,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       4,     1,     1,     1,     1,     1,     1
  };


#if YYDEBUG || 1
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const  Parser ::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "\"if\"",
  "\"iffalse\"", "\"goto\"", "\"halt\"", "\"call\"", "\"putparam\"",
  "\"getparam\"", "\"nop\"", "\"return\"", "\"print\"", "\"newline\"",
  "\"block\"", "\"=\"", "\"+\"", "\"-\"", "\"*\"", "\"/\"", "\"&\"",
  "\"&&\"", "\"||\"", "\"<\"", "\"<=\"", "\">\"", "\">=\"", "\"==\"",
  "\"!=\"", "\"(\"", "\")\"", "\"[\"", "\"]\"", "\";\"", "\":\"", "\",\"",
  "\".\"", "\"identifier\"", "\"string\"", "\"bool\"", "\"char\"",
  "\"int\"", "\"float\"", "\"print_to_console\"", "$accept", "dest",
  "value", "quadruple", "if_statement", "goto", "assignment",
  "array_assignment", "var_declaration", "label", "term", "program",
  "statement", "mb_newline", "newlines", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const unsigned char
   Parser ::yyrline_[] =
  {
       0,    93,    93,    94,    98,    99,   101,   101,   102,   102,
     102,   105,   106,   107,   108,   109,   110,   111,   112,   115,
     116,   117,   121,   126,   134,   135,   138,   143,   145,   149,
     153,   154,   158,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   185,   186,   187,   188,   189,   190
  };

  void
   Parser ::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
   Parser ::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG


} // yy
#line 1701 "../src/utilities/parser/compiled_parser/parser.cpp"

#line 193 "/mnt/d/programming/c/tac_parser/src/toy_optimizer/src/utilities/parser/grammar/parser.y"


#include <sstream>
std::string g_bison_error_msg;
void yy::Parser::error(const location_type& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;

    std::stringstream s;
    s << l << ": " << m;
    g_bison_error_msg = s.str();
}
