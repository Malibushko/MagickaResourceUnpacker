#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "Unpacker.h"

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Expected 2 arguments" << std::endl;
        return 1;
    }

    Unpacker unpacker;
    unpacker.UnpackDirectory(argv[1]);

    return 0;
}
