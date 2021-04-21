

#include "tac_project.h"

// ------- Flex/Bison routine ----
//#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yylex();
extern int yyparse();
// -----------------------------

extern Node *g_tree_root;

std::string g_syntax_errors;
std::string g_warnings;

bool interpret_string(std::string code) {
    if (code.back() != '\n')
        code += '\n';

    Interpreter interpreter;
    interpreter.reset(g_tree_root);

//    std::cout << "Code: " << code.c_str() << std::endl;
    
    YY_BUFFER_STATE buffer = yy_scan_string(code.c_str());
    yyparse();
    yy_delete_buffer(buffer);

    if (!g_tree_root) {
        printf("Tree root node is incorrect. Parsing error! \n");
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

    auto main_function = "main";
    auto start_node = find_label_node(main_function);
    if (!start_node) {
        printf("MAIN isn't defined.\n");
        return false;
    }


//    print_tree(stdout, g_tree_root, 0);
    std::cout << "--> Simulator started..." << std::endl;
    interpreter.begin_interpretation(start_node, "main");
    std::cout << "--> Simulator finished" << std::endl;

    if (!g_syntax_errors.empty()) {
        printf("%s", g_syntax_errors.c_str());
    }

    return true;
}

