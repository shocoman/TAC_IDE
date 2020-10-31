#include <iostream>
#include <numeric>

#include "driver/driver.hpp"
#include "tac_worker/dataflow_graph.hpp"

int main(int argc, char *argv[]) {

//
//    auto q = Quad("1", "2", Quad::Type::Add);
//    auto prev = q;
//
//    std::cout << "Before First: " << q.fmt() << std::endl;
//    std::cout << "Before Second: " << prev.fmt() << std::endl;
//    std::cout << "Equal?: " << (q == prev) << std::endl;
//    constant_folding(q);
//    std::cout << "After First: " << q.fmt() << std::endl;
//    std::cout << "After Second: " << prev.fmt() << std::endl;
//
//    std::cout << "Equal?: " << (q == prev) << std::endl;
//
//
//    exit(22);

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

//    drv.parse("../myfile");
    drv.parse("../myfile2");

    make_cfg(std::move(drv.labels), std::move(drv.quadruples));

    return 0;
}



