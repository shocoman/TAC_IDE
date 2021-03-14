
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <thread>

#include "interpreter.h"
#include "semantic_analysis.h"

//********** Flex/Bison routine
//#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yylex();
extern int yyparse();
////////////////////////////////

void reset_interpreter();
extern Node *g_tree_root;

string g_syntax_errors;
string g_warnings;

extern std::condition_variable g_cond_var;
extern std::mutex g_mutex;

bool interpret_string(std::string code) {
    if (code[code.size() - 1] != '\n')
        code += '\n';

    Interpreter interpreter;
    interpreter.reset(g_tree_root);

    std::cout << "Code: " << code.c_str() << std::endl;
    
    YY_BUFFER_STATE buffer = yy_scan_string(code.c_str());
    yyparse();
    yy_delete_buffer(buffer);

    if (!g_tree_root) {
        printf("Tree root node is incorrect.\n");
        return false;
    }
    set_line_numbers_for_all_instructions(g_tree_root);
    semantic_analysis(g_tree_root);

    if (!g_warnings.empty()) {
        printf("%s", g_warnings.c_str());
        printf("-------------------------------------------------\n");
    }
    if (!g_syntax_errors.empty()) {
        printf("%s", g_syntax_errors.c_str());
        return false;
    }

    auto start_node = find_label_node("main");
    if (!start_node) {
        printf("MAIN isn't defined.\n");
        return false;
    }


//    print_tree(stdout, g_tree_root, 0);
    interpreter.begin_interpretation(start_node, "main");

    if (!g_syntax_errors.empty()) {
        printf("%s", g_syntax_errors.c_str());
    }

    return true;
}

void prepare_interpreter_for_string(std::string code) {
    if (code[code.size() - 1] != '\n')
        code += '\n';

    Interpreter interpreter;
    interpreter.reset(g_tree_root);

    YY_BUFFER_STATE buffer = yy_scan_string(code.c_str());
    yyparse();
    yy_delete_buffer(buffer);

    set_line_numbers_for_all_instructions(g_tree_root);
    semantic_analysis(g_tree_root);

    auto start_node = find_label_node("main");
    interpreter.begin_interpretation(start_node, "main");
}



int main(int argc, char *argv[]) {
    bool check_syntax = false;
    bool show_tac_tree = false;

    if (argc < 2 || argc > 4 || argc == 2 && !strcmp(argv[1], "-h")) {
        if (argc < 2)
            printf("Too few parameters in command line.\n");
        else if (argc > 4)
            printf("Too much parameters in command line.\n");

        printf("-------------------------------------\n");
        printf("interpreter.exe <parameters> <filename>\n\nParameters:\n");
        printf("%-13s%s\n", "-h", "Display help and exit");
        printf("%-13s%s\n", "-file", "Display g_tree_root");
        printf("%-13s%s\n", "-s", "Check syntax only");
        return -1;
    }

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-file"))
            show_tac_tree = true;
        if (!strcmp(argv[i], "-s"))
            check_syntax = true;
    }

    // Читаем файл с кодом и конвертируем в строку
    std::ifstream file(argv[argc - 1]);
    if (!file.is_open()) {
        std::cout << "Couldn't open file '" << argv[argc - 1] << "'" << std::endl;
        return -1;
    }

    std::stringstream file_buffer;
    file_buffer << file.rdbuf();
    file.close();

    //Парсим строку. Строим дерево.
    auto res = interpret_string(file_buffer.str());
    if (!res)
        return -1;

    //Выводим дерево при необходимости.
    if (show_tac_tree) {
        printf("3ac-g_tree_root:\n");
        print_tree(stdout, g_tree_root, 0);
        printf("-------------------------------------------------\n");
    }

    return 0;
}
