//
// Created by victor on 03.02.2021.
//

#ifndef TAC_SIMULATOR_TEST_CONVERTDOTTOIMAGE_HPP
#define TAC_SIMULATOR_TEST_CONVERTDOTTOIMAGE_HPP

#include <string>
#include <iostream>
#include <vector>

#include <graphviz/gvc.h>

std::vector<char> renderGraphFromString(const std::string& dot_content);

#endif   // TAC_SIMULATOR_TEST_CONVERTDOTTOIMAGE_HPP
