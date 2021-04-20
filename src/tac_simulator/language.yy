/**
 * language.yy
 *   Синтаксический анализатор очень простого языка,
 *   выводящий на экран дерево разбора.
 *
 */

%no-lines
%verbose
%define parse.error verbose
%require "2.4"

%{
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <list>

using namespace std;
#include "tac_tree.h"
#include "label_table.h"
#include "symbol_table.h"

%}


// +----------------------------+
// | Определения для Bison/Yacc |
// +----------------------------+

/* Поскольку мы пытаемся построить дерево разбора,
 * нужно, чтобы с каждым элементом была связана часть дерева.
 */
%{
    #define YYSTYPE Node*
    #define YYINITDEPTH 1000
%}

/* Таблица лексем. */
%token-table

/* Все лексемы, которые используются. */
%token TOKENS_START

%token _ID
%token _IDTEMP

%token ','
%token ':'
%token '('
%token ')'
%token '{'
%token '}'
%token '['
%token ']'

%token _DCONST
%token _DCONSTE
%token _ICONST
%token _CCONST
%token _SCONST

%token _ARRAYTYPE 
%token _BYTETYPE 
%token _WORDTYPE 
%token _LONGTYPE 
%token _DOUBLETYPE 
%token _ASCIITYPE 

%token _BYTEARRAYTYPE
%token _WORDARRAYTYPE
%token _LONGARRAYTYPE
%token _DOUBLEARRAYTYPE
%token _ASCIIARRAYTYPE

%token _TOASCII
%token _TOBYTE
%token _TOWORD
%token _TOLONG
%token _TODOUBLE

%token _IFTRUE 
%token _IFFALSE 
%token _GOTO 
%token _PUTPARAM 
%token _GETPARAM 
%token _CALL 
%token _RETURN 
%token _IWRITE 
%token _FWRITE 
%token _CWRITE 
%token _SWRITE 
%token _IREAD 
%token _FREAD 
%token _CREAD 
%token _SREAD

%token _SEPARATOR

%token TOKENS_END

%left '<'
%left '>'
%left _EQ
%left _NEQ
%left _GEQ
%left _LEQ

%left _AND
%left _OR
%left '^'
%left '!'

%right '='
%right _IS
%left '+' '-'
%left '*' '/' '%'

%nonassoc UMINUS
%nonassoc UOPNOT

%{

/* Несколько внешних программных объектов от лексера, которые нужно предварительно объявить. */
extern char* yytext;
extern int yylineno;

extern string g_syntax_errors;

extern int yylex(void);

/* Определяем функцию yyerror. */
void yyerror(const char* err) {
	string error = 	"Error! Line " + to_string(yylineno) + " near <" + string(yytext) + "> - " + string(err) + "\n";
	g_syntax_errors += error;

    // Не очень хорошо поступаем, не освобождая память перед выходом.
    // Для завершения работы после первой ошибки нужно раскомментировать код на следующей строке.
    // exit (1);
}

void yyerror(const char* err, string name) {
	g_syntax_errors += "Error! <" + name + "> - " + to_string(yylineno) + " <=> " + string(err) + "\n";
}

%}

// +-------------------------------+
// | Глобальные объекты Bison/Yacc |
// +-------------------------------+
%{
Node* g_tree_root;
list<Node*> stList;
list<Node*> exList;
list<Node*> tempList;
list<Node*> indexList;
%}

// +----------------------+
// | Продукции грамматики |
// +----------------------+

%%
start : program 
	{ 
		stList.clear();
		exList.clear();
		tempList.clear();
		indexList.clear();
	}
	;

program : statement
	{ 
		$$ = create_node("program", UndefType, stList);
		stList.clear();
		g_tree_root = $$;
	}
	;

statement : /* epsilon-правило */
    {
		$$=NULL;
	}
	|  _SEPARATOR statement /* epsilon-правило + левый разделитель */
    {
		$$=NULL;
	}

/*-------------------------------------------------------------------*/
/*Метка*/
/*-------------------------------------------------------------------*/
    /* метка и идущие после неё выражения, крайний стейтмент - другая метка или пусто, то есть окончание */
	| ident ':' _SEPARATOR statement
	{
		Node* lb = $1;
		Node* n = create_node(lb->value, LabelType, exList);
		exList.clear();
		stList.push_back(n);
		
		/*Здесь должен быть код добавления метки в таблицу меток*/
		if(find_label(lb->value)!=NULL)
		{
			yyerror("Redefinition of label.", lb->value);
		}
		else
			add_label(lb->value, n);
		
		$$ = n;		
	}
/*-------------------------------------------------------------------*/
/*Объявления массивов*/
/*-------------------------------------------------------------------*/
	| ident ':' _ARRAYTYPE operand ',' _BYTEARRAYTYPE ',' operand _SEPARATOR statement
	{
		tempList.push_front($1);
		tempList.push_front($4);
		tempList.push_front($8);
		$$ = create_node(".block .byte", CharType, tempList);
		tempList.clear();
		
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
		{
			add_symbol($1->value, CharSymbolType, $8->value, $4->value);
		}
		exList.push_back($$);
	}
	| ident ':' _ARRAYTYPE operand ',' _WORDARRAYTYPE ',' operand _SEPARATOR statement
	{
		tempList.push_front($1);
		tempList.push_front($4);
		tempList.push_front($8);
		$$ = create_node(".block .word", IntType, tempList);
		tempList.clear();
		
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
		{
			add_symbol($1->value, WordSymbolType, $8->value, $4->value);
		}
		exList.push_back($$);
	}
	| ident ':' _ARRAYTYPE operand ',' _LONGARRAYTYPE ',' operand _SEPARATOR statement
	{
		tempList.push_front($1);
		tempList.push_front($4);
		tempList.push_front($8);
		$$ = create_node(".block .long", IntType, tempList);
		tempList.clear();
		
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
		{
			add_symbol($1->value, LongSymbolType, $8->value, $4->value);
		}
		exList.push_back($$);
	}
	| ident ':' _ARRAYTYPE operand ',' _DOUBLEARRAYTYPE ',' operand _SEPARATOR statement
	{
		tempList.push_front($1);
		tempList.push_front($4);
		tempList.push_front($8);
		$$ = create_node(".block .double", DoubleType, tempList);
		tempList.clear();

		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
		{
			add_symbol($1->value, DoubleSymbolType, $8->value, $4->value);
		}
		exList.push_back($$);
	}
	| ident ':' _ARRAYTYPE operand ',' _ASCIIARRAYTYPE ',' operand _SEPARATOR statement
	{
		tempList.push_front($1);
		tempList.push_front($4);
		tempList.push_front($8);
		$$ = create_node(".block .ascii", StringType, tempList);
		tempList.clear();
		
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
		{
			add_symbol($1->value, StringSymbolType, $8->value, $4->value);
		}
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Объявления переменных*/
/*-------------------------------------------------------------------*/
    | ident ':' _WORDTYPE iconst _SEPARATOR statement
	{
		$$ = create_node(".word", IntType, $1, $4);
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
			add_symbol($1->value, WordSymbolType, $4->value, NULL);
		exList.push_back($$);
	}
	| ident ':' _LONGTYPE iconst _SEPARATOR statement
	{
		$$ = create_node(".long", IntType, $1, $4);
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
			add_symbol($1->value, LongSymbolType, $4->value, NULL);
		exList.push_back($$);
	}
	| ident ':' _DOUBLETYPE dconst _SEPARATOR statement
	{
		$$ = create_node(".double", DoubleType, $1, $4);
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
			add_symbol($1->value, DoubleSymbolType, $4->value, NULL);
		exList.push_back($$);
	}
	| ident ':' _BYTETYPE cconst _SEPARATOR statement
	{
		$$ = create_node(".byte", CharType, $1, $4);
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
			add_symbol($1->value, CharSymbolType, $4->value, NULL);
		exList.push_back($$);
	}
	| ident ':' _ASCIITYPE sconst _SEPARATOR statement
	{
		$$ = create_node(".ascii", StringType, $1, $4);
		if(find_symbol($1->value)!=NULL)
		{
			yyerror("Redefinition of symbol.", $1->value);
		}
		else
			add_symbol($1->value, StringSymbolType, $4->value, NULL);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Присваивания идентификатору*/
/*-------------------------------------------------------------------*/
	| ident _IS binop _SEPARATOR statement
	{
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| ident _IS operand _SEPARATOR statement
	{
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| ident _IS unops _SEPARATOR statement
	{ 
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| ident _IS arrunops _SEPARATOR statement
	{ 
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| ident _IS arrayelement _SEPARATOR statement
	{
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Присваивания элементу массива*/
/*-------------------------------------------------------------------*/	
	| arrayelement _IS unops _SEPARATOR statement
	{
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| arrayelement _IS operand _SEPARATOR statement
	{
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	
/*-------------------------------------------------------------------*/
/*Присваивания временной переменной*/
/*-------------------------------------------------------------------*/
	| tident _IS binop _SEPARATOR statement
	{
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| tident _IS operand _SEPARATOR statement
	{
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| tident _IS unops _SEPARATOR statement
	{ 
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| tident _IS arrunops _SEPARATOR statement
	{ 
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	| tident _IS arrayelement _SEPARATOR statement
	{
		$$ = create_node(":=", UndefType, $1, $3);
		exList.push_back($$);
	}
	
/*-------------------------------------------------------------------*/
/*Передача параметров*/
/*-------------------------------------------------------------------*/
	| _PUTPARAM operand _SEPARATOR statement
	{
		$$ = create_node("putparam", UndefType, $2, NULL);
		exList.push_back($$);
	}
	| _PUTPARAM arrayelement _SEPARATOR statement
	{
		$$ = create_node("putparam", UndefType, $2, NULL);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Вывод на экран*/
/*-------------------------------------------------------------------*/
	| _CALL wfunc ',' iconst _SEPARATOR statement
	{
		$$ = create_node("call", UndefType, $2, $4);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Ввод данных*/
/*-------------------------------------------------------------------*/
	| tident _IS _CALL rfunc ',' iconst _SEPARATOR statement
	{
		if(strcmp($6->value, "0")!=0)
		{
			yyerror("Unexpected _ICONST. Expected 0.", $6->value);
		}
		Node* tn = create_node("call", UndefType, $4, $6);
		$$ = create_node(":=", UndefType, $1, tn);
		exList.push_back($$);
	}
	| ident _IS _CALL rfunc ',' iconst _SEPARATOR statement
	{
		if(strcmp($6->value, "0")!=0)
		{
			yyerror("Unexpected _ICONST. Expected 0.", $6->value);
		}
		Node* tn = create_node("call", UndefType, $4, $6);
		$$ = create_node(":=", UndefType, $1, tn);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Преобразование*/
/*-------------------------------------------------------------------*/
	| tident _IS _CALL convfunc ',' iconst _SEPARATOR statement
	{
		Node* tn = create_node("call", UndefType, $4, $6);
		$$ = create_node(":=", UndefType, $1, tn);
		exList.push_back($$);
	}
	| ident _IS _CALL convfunc ',' iconst _SEPARATOR statement
	{
		Node* tn = create_node("call", UndefType, $4, $6);
		$$ = create_node(":=", UndefType, $1, tn);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Вызов пользовательских функций с присваиванием и без*/
/*-------------------------------------------------------------------*/
	| tident _IS _CALL ident ',' iconst _SEPARATOR statement
	{
		Node* tn = create_node("call", UndefType, $4, $6);
		$$ = create_node(":=", UndefType, $1, tn);
		exList.push_back($$);
	}
	| ident _IS _CALL ident ',' iconst _SEPARATOR statement
	{
		Node* tn = create_node("call", UndefType, $4, $6);
		$$ = create_node(":=", UndefType, $1, tn);
		exList.push_back($$);
	}
	| _CALL ident ',' iconst _SEPARATOR statement
	{
		$$ = create_node("call", UndefType, $2, $4);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Безусловный переход*/
/*-------------------------------------------------------------------*/
	| _GOTO ident _SEPARATOR statement
	{
		$$ = create_node("goto", UndefType, $2, NULL);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Получение параметров*/
/*-------------------------------------------------------------------*/
	| _GETPARAM ident _SEPARATOR statement
	{
		$$ = create_node("getparam", UndefType, $2, NULL);
		exList.push_back($$);
	}
	| _GETPARAM tident _SEPARATOR statement
	{
		$$ = create_node("getparam", UndefType, $2, NULL);
		exList.push_back($$);
	}
	
	| _GETPARAM arrayelement _SEPARATOR statement
	{
		$$ = create_node("getparam", UndefType, $2, NULL);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Возврат результата функции*/
/*-------------------------------------------------------------------*/
	| _RETURN operand _SEPARATOR statement
	{
		$$ = create_node("return", UndefType, $2, NULL);
		exList.push_back($$);
	}
	| _RETURN arrayelement _SEPARATOR statement
	{
		$$ = create_node("return", UndefType, $2, NULL);
		exList.push_back($$);
	}
/*-------------------------------------------------------------------*/
/*Условные*/
/*-------------------------------------------------------------------*/
	| _IFFALSE ident _GOTO ident _SEPARATOR statement
	{
		$$ = create_node("iffalse", UndefType, $2, $4);
		exList.push_back($$);
	}
	| _IFTRUE ident _GOTO ident _SEPARATOR statement
	{
		$$ = create_node("iftrue", UndefType, $2, $4);
		exList.push_back($$);
	}
	| _IFFALSE tident _GOTO ident _SEPARATOR statement
	{
		$$ = create_node("iffalse", UndefType, $2, $4);
		exList.push_back($$);
	}
	| _IFTRUE tident _GOTO ident _SEPARATOR statement
	{
		$$ = create_node("iftrue", UndefType, $2, $4);
		exList.push_back($$);
	}
	| _IFFALSE arrayelement _GOTO ident _SEPARATOR statement
	{
		$$ = create_node("iffalse", UndefType, $2, $4);
		exList.push_back($$);
	}
	| _IFTRUE arrayelement _GOTO ident _SEPARATOR statement
	{
		$$ = create_node("iftrue", UndefType, $2, $4);
		exList.push_back($$);
	}
    ;
	
/*-------------------------------------------------------------------*/
/*Бинарные операции*/
/*-------------------------------------------------------------------*/

binop
    : operand '+' operand   { $$ = create_node("+", UndefType, $1, $3); }
	| operand '-' operand   { $$ = create_node("-", UndefType, $1, $3); }
	| operand '*' operand   { $$ = create_node("*", UndefType, $1, $3); }
	| operand '/' operand   { $$ = create_node("/", UndefType, $1, $3); }
	| operand '%' operand   { $$ = create_node("%", UndefType, $1, $3); }
	| operand '>' operand   { $$ = create_node(">", UndefType, $1, $3); }
	| operand '<' operand   { $$ = create_node("<", UndefType, $1, $3); }
	| operand '^' operand   { $$ = create_node("^", UndefType, $1, $3); }
	| operand _GEQ operand  { $$ = create_node(">=", UndefType, $1, $3); }
	| operand _LEQ operand  { $$ = create_node("<=", UndefType, $1, $3); }
	| operand _EQ operand   { $$ = create_node("==", UndefType, $1, $3); }
	| operand _NEQ operand  { $$ = create_node("!=", UndefType, $1, $3); }
	| operand _AND operand  { $$ = create_node("&&", UndefType, $1, $3); }
	| operand _OR operand   { $$ = create_node("||", UndefType, $1, $3); }
	;

/*-------------------------------------------------------------------*/
/*Унарные операции*/
/*-------------------------------------------------------------------*/
unops
    : '-' operand { $$ = create_node("-", UndefType, $2, NULL); }
	| '!' operand { $$ = create_node("!", UndefType, $2, NULL); }
	;
arrunops
    : '-' arrayelement { $$ = create_node("-", UndefType, $2, NULL); }
	| '!' arrayelement { $$ = create_node("!", UndefType, $2, NULL); }
	;
	
/*-------------------------------------------------------------------*/
/*Функции*/
/*-------------------------------------------------------------------*/
convfunc : _TOASCII
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _TOBYTE
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _TODOUBLE
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _TOLONG
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _TOWORD
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	;
	
	
wfunc : _IWRITE
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _FWRITE
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _CWRITE
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _SWRITE
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	;
	
rfunc : _IREAD
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _FREAD
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _CREAD
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	| _SREAD
	{
		std::string name(yytext);
		$$=create_node(name.c_str(), UndefType);
	}
	;
/*-------------------------------------------------------------------*/
/*Операнды*/
/*-------------------------------------------------------------------*/	

arrayelement :  ident '[' iconst ']'
	{
		indexList.push_front($3);
		$$ = create_node($1->value, IdentType, indexList);
		indexList.clear();
	}
	| ident '[' ident ']'
	{
		indexList.push_front($3);
		$$ = create_node($1->value, IdentType, indexList);
		indexList.clear();
	}
	| ident '[' tident ']'
	{
		indexList.push_front($3);
		$$ = create_node($1->value, IdentType, indexList);
		indexList.clear();
	}
	;
	
ident : _ID { $$=create_node(yytext, IdentType); };
	
tident : _IDTEMP { $$=create_node(yytext, IdentType); };

iconst : _ICONST { $$=create_node(yytext, IntType); };

dconst
    : _DCONST { $$=create_node(yytext, DoubleType); }
	| _DCONSTE { $$=create_node(yytext, DoubleType); };

cconst : _CCONST
    {
		string sname = string(yytext);
		sname = sname.substr(1, sname.length()-2);
		yyval=create_node(sname.c_str(), CharType);
	};

sconst : _SCONST
	{
		string sname = string(yytext);
		sname = sname.substr(1, sname.length()-2);
		yyval=create_node(sname.c_str(), StringType);
	};

	
operand
    : ident
	| tident
	|_CCONST
	{
		string sname = string(yytext);
		sname = sname.substr(1, sname.length()-2);
		yyval = create_node(sname.c_str(), CharType);
	}
	| _SCONST
	{
		string string_name = string(yytext);
		string_name = string_name.substr(1, string_name.length()-2);
		yyval = create_node(string_name.c_str(), StringType);
	}
	| _ICONST   { $$=create_node(yytext, IntType); }
	| _DCONST   { $$=create_node(yytext, DoubleType); }
	| _DCONSTE  { $$=create_node(yytext, DoubleType); };
%%
