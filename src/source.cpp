#include "../includes/json.hpp"
#include "configurable_parser.h"
#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

using json = nlohmann::json;

using namespace std;

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); 
    SetConsoleCP(CP_UTF8);  
#endif

    cout << "\n";
    Parser parser;
    bool parallel_mode = false;
    string config_path = "test/example_test/configs/example.json";
    string data_path = "test/example_test/sensor_data/";
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
                 << "  --config <path>     Path to config file (default: test/example_test/configs/example.json)\n"
                 << "  --data <path>       Path to data directory (default: test/example_test/sensor_data/)\n"
                 << "  --help, -h          Show this help message\n";
            return 0;
        }
    }

    try {
        bool configure_complete = parser.Configure(config_path);
        if (!configure_complete) {
            std::cerr << "Couldn't find config file, aborting\n";
            return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return -1;
    }
    parser.Run(data_path, std::cout, parallel_mode);
    return 0;
}