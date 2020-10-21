#include <iostream>

#include "driver/driver.hpp"
#include "tac_worker/dataflow_graph.hpp"

int main(int argc, char *argv[]) {




    int res = 0;
    driver drv;
//    for (int i = 1; i < argc; ++i)
//        if (argv[i] == std::string ("-p"))
//            drv.trace_parsing = true;
//        else if (argv[i] == std::string ("-s"))
//            drv.trace_scanning = true;
//        else if (!drv.parse (argv[i]))
//            std::cout << "Parsing result: " << drv.result << '\n';
//        else
//            res = 1;

    drv.parse("../myfile");

    make_cfg(std::move(drv.labels), std::move(drv.quadruples));

    return 0;
}



