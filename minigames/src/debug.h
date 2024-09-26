
#define DEBUG

#include <iostream>
#ifdef DEBUG 
#define D(x) std::cout << "DEBUG: " << x << std::endl;
#else 
#define D(x)
#endif
