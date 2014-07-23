//
//  main.cpp
//  rdvlisp
//
//  Created by Ruben De Visscher on 25/06/14.
//  Copyright (c) 2014 Ruben De Visscher. All rights reserved.
//

#include <iostream>
#include <string>
#include "reader.h"

std::vector<std::istream::streampos> line_start_positions(std::istream& is) {
    auto initial = is.tellg();
    is.seekg(0);
    std::vector<std::istream::streampos> positions;
    if(is.good()) {
        positions.push_back(is.tellg());
    }
    while(is.good()) {
        if(is.get() == '\n') {
            positions.push_back(is.tellg());
        }
    }
    is.seekg(initial);
    return positions;
}

int main(int argc, const char * argv[])
{
    std::string s("(print-ln \"Hello, World!\\n\" :newline! 1.23e-23 2345 -0)");
        auto result = rdvlisp::read(s);
        if(result.good()) {
            std::cout << *result.get() << std::endl;
        } else if(result.end != 0) {
            std::cerr << "Error " << rdvlisp::read_error(result.error(), result.start, result.end).what() << std::endl;
        }
    return 0;
}

