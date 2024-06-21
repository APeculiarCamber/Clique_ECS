#pragma once
#include <iostream>

#define PDEBUG
#ifndef PDEBUG
#define PRINT(msg) std::cout << msg
#else
#define PRINT(msg) ((void)0)
#endif


#ifndef PDEBUG
#define PRINT_FOLD(msg) ((std::cout << msg), ...)
#else
#define PRINT_FOLD(msg) ((void)0)
#endif