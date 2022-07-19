/**
 * @file main.cpp
 * @author Eron Ristich
 * @brief Initializes and handles application window
 * @version 0.1
 * @date 2022-05-29
 * 
 * @copyright Copyright (c) 2022
 */

#include "kernel/kernel.h"

#include <iostream>

int main(int argc, char* argv[]) {

    std::cout << "Reached line " << __LINE__ << " in " << __FILE__ << std::endl;

    Kernel* kernel = new Kernel();

    std::cout << "Reached line " << __LINE__ << " in " << __FILE__ << std::endl;
    kernel->start(string("Window"), 700, 700);

    std::cout << "Reached line " << __LINE__ << " in " << __FILE__ << std::endl;
    
    delete kernel;
    return 0;
}