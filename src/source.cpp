#include "../includes/json.hpp"
#include <iostream>
#include <fstream>
#include <windows.h>

using json = nlohmann::json;

int main() {
    SetConsoleOutputCP(CP_UTF8); 
    SetConsoleCP(CP_UTF8);  

    std::ifstream f("test/example.json");
    std::cout << f.is_open()<<'\n';
    
    json data = json::parse(f);
    std::cout << data.dump() << "\n####\n";
    std::cout << "hedыыыыыы";
    return 0;
}