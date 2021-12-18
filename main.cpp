#include <iostream>
#include <string>
#include <fstream>
#include <tuple>
#include "CSVParser.h"

int main()
{
    std::ifstream input("input.txt");
    CSVParser<size_t, int, std::string> parser;
    parser.Pars(input, 1);
    for (std::tuple<size_t, int, std::string> tp : parser)
    {
        std::cout << tp;
        std::cout << std::endl;
    }
    std::cout << parser.GetRow(0);
    std::cout << std::endl << parser.GetRow(1);
}
