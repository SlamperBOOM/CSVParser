#include <iostream>
#include <string>
#include <fstream>
#include <tuple>
#include "CSVParser.h"

int main()
{
    std::ifstream input("input.txt");
    CSVParser<size_t, int, std::string> parser(input, 1);
    for (std::tuple<size_t, int, std::string> tp : parser)
    {
        std::cout << tp;
        std::cout << std::endl;
    }
}
