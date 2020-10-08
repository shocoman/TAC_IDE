all: gen_parser gen_lexer
	g++ -o parser main.cpp parser/driver/driver.cpp parser/parser.cpp parser/lexer.cpp

gen_lexer:
	flex --wincompat -o parser/lexer.cpp parser/grammar/lexer.l

gen_parser:
	bison parser/grammar/parser.y -d -o parser/parser.cpp
