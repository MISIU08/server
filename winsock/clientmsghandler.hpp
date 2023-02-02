#include <iostream>
#include<windows.h>


int handlemsg(std::string Buffer){
    std::cout << "Recv: " << Buffer << std::endl;
    return 0;
}