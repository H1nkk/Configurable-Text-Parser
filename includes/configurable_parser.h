#pragma once
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <iostream>
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

// TODO: добавить всякие проверки в Parser::Configure

class Parser {
private:
    enum types {
        kBool = 0,
        kFloat = 1,
        kUnsignedLL = 2
    };

    struct SensorData {
        std::string infile_sensor_name_;
        std::string output_sensor_name_;
        std::map<std::string, std::variant<bool, float, unsigned long long>> property_to_value_;
    };

    struct FileData {
        std::string file_name_;
        std::vector<SensorData>  sensors_;
    };

    class Rule {
        std::map<std::string, std::string> properties_;
    public:
        std::string& operator[](std::string propetry_name) {
            return properties_[propetry_name];
        }

        friend std::ostream& operator<<(std::ostream& os, const Rule& rule) {
            for (auto& [key, value] : rule.properties_) {
                os << "    " << key << ": " << value << '\n'; 
            }
            return os;
        }
    };

    struct Extractor {
        std::string name_;
        std::vector<std::string> rules_to_extract_;
    };
    
    struct Config {
        std::map<std::string, std::string> sensors_rule_to_name_;
        std::map<std::string, Rule> rules_;
        std::vector<Extractor> extractors_;

        void clear() {
            sensors_rule_to_name_.clear();
            rules_.clear();
            extractors_.clear();
        }

        void dump(std::ostream& os) {
            os << "sensors_rule_to_name_:\n";
            for (auto& [key, value] : sensors_rule_to_name_) {
                os << "  " << key << ": " << value << '\n';
            }

            os << "rules_:\n";
            for (auto& [key, rule] : rules_) {
                os << "  " << key << ":\n";
                os << rule;
            }

            os << "extractors_:\n";
            for (auto& extractor : extractors_) {
                
                os << "  " << extractor.name_ << ":\n";
                for (auto& rule_to_extract : extractor.rules_to_extract_) {
                    os << "    " << rule_to_extract << "\n";
                }
            }
        }
    };
    
    Config config_;
    std::vector<FileData> files_data_;

    FileData ParseFile(const std::string& path_to_file) {

    }
    void Parse() { // заполняет files_data
        
    }
    void Analyze() {

    }

public:
    bool Configure(const std::string& path_to_config_file) { // заполняет config_
        // сначала очищаем config_
        config_.clear();

        std::ifstream f(path_to_config_file);

        json data = json::parse(f);

        for (auto& sensor : data["sensors"]) {
            std::cout << sensor["rule"] << ": " << sensor["name"] << '\n';
            config_.sensors_rule_to_name_[sensor["rule"]] = sensor["name"];
        }

        for (auto& rule : data["rules"]) {
            auto& properties = config_.rules_[rule["name"]];
            for (auto& property : rule.items()) {
                if (property.key() == "name") {
                    continue;
                }
                properties[property.key()] = property.value();
            }
            std::cout << "#####\n\n";
        }

        for (auto& extractor : data["extractors"]) {
            Extractor new_extractor;
            new_extractor.name_ = extractor["sensor"];
            for (auto& rule_name : extractor["rules"]) {
                new_extractor.rules_to_extract_.push_back(rule_name);
            }
            config_.extractors_.push_back(std::move(new_extractor));
        }


        std::cout << "\nconfig.dump:\n";
        config_.dump(std::cout);
        return true;
    }

    void Run(std::string path_to_files, std::ostream& os = std::cout) { // тут запускается Parse, затем Analyze
        Parse();
        Analyze();
    }
};