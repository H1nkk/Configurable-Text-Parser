#include "../includes/json.hpp"
#include "configurable_parser.h"
#include <iostream>
#include <fstream>
#include <windows.h>

using json = nlohmann::json;

using namespace std;

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8); 
    SetConsoleCP(CP_UTF8);  

    cout << "\n";
    Parser parser;
    bool parallel_mode = false;
    string config_path = "test/configs/example.json";
    string data_path = "test/sensor_data/";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--parallel") == 0) {
            parallel_mode = true;
        }
        else if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            config_path = argv[++i];
        }
        else if (strcmp(argv[i], "--data") == 0 && i + 1 < argc) {
            data_path = argv[++i];
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            cout << "Usage: " << argv[0] << " [options]\n"
                 << "Options:\n"
                 << "  --parallel          Enable parallel parsing mode\n"
                 << "  --config <path>     Path to config file (default: test/configs/example.json)\n"
                 << "  --data <path>       Path to data directory (default: test/sensor_data/)\n"
                 << "  --help, -h          Show this help message\n";
            return 0;
        }
    }

    parser.Configure(config_path);
    parser.Run(data_path, std::cout, parallel_mode);
    return 0;
}