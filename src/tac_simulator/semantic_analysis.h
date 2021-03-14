#include "interpreter.h"
#include "tac_tree.h"

bool semantic_analysis(Node* tree);
//Получение типа результата бинарной/унарной операции (нах.справа)
SymbolDataType get_operation_data_type(Node* n);
//Получение типа данных передаваемых "справа" (константа или переменная)
SymbolDataType get_operand_type(Node* n);
bool change_temp_datatype(Node* n);