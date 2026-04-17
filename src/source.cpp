#include "../includes/json.hpp"
#include "configurable_parser.h"
#include <iostream>
#include <fstream>
#include <windows.h>

using json = nlohmann::json;

using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8); 
    SetConsoleCP(CP_UTF8);  

    cout << "\n";
    Parser parser;
    parser.Configure("test/example.json");
    return 0;
}