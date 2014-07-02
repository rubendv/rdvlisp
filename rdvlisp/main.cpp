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
    std::stringstream ss("#(test) #() #test (test '(+ a ,,b))");
    while(ss.good()) {
        auto result = rdvlisp::read(ss);
        if(result.good()) {
            std::cout << *result.get() << std::endl;
        } else if(result.start != -1) {
            std::cerr << "Error " << rdvlisp::read_error(result.error(), result.start, result.end).what() << std::endl;
            break;
        }
    }
    return 0;
}

