//
// Created by victor on 19.04.2021.
//

#include "tac_project.h"



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
