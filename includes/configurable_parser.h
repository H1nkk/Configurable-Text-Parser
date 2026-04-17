#pragma once
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <iostream>
#include <fstream>
#include <json.hpp>
#include <filesystem>

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
        std::vector<SensorData> sensors_;

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

    FileData ParseFile(const std::filesystem::path& path_to_file) {
        // TODO переделать составление result (заменить на конструктор)
        std::string file_name = path_to_file.filename().string();
        std::ifstream data_file_ifstream(path_to_file);
        if (!data_file_ifstream.is_open()) {
            throw "Couldn't open " + path_to_file.string() + " file."; // TODO обернуть в красивый тип исключения
        }

        std::string line;
        
        while (std::getline(data_file_ifstream, line)) {
            // убираем ведущие и последние незначащие пробелы
            try {
                line = line.substr(line.find_first_not_of(" \t\n\r"), line.find_last_not_of(" \t\n\r"));
            }
            catch (std::exception &e) {
                // TODO если мы здесь, значит line == "". надо делать просто скип
                std::cout << e.what();
            }
            std::cout << "line: " << line << '\n';

        }

        FileData result;
        result.file_name_ = file_name;
        return result;
    }
    void Parse(const std::string& path_to_files) { // заполняет files_data
        // тут можно сделать проверку, что есть хотя бы один файл для парсинга
        // ещё сюда можно добавить параллельности
        std::filesystem::path test{path_to_files};
        for (auto const& dir_entry : std::filesystem::directory_iterator{test}) {
            if (dir_entry.is_regular_file()) {
                std::filesystem::path file_path = dir_entry.path();
                if (file_path.extension() == ".txt") {
                    files_data_.push_back(ParseFile(file_path));
                }
            }
        }
    }
    void Analyze() {

    }

public:
    /// @brief configures parser based on config file
    /// @param path_to_config_file путь до файла конфигурации
    /// @return false если открыть файл не удалось, иначе true
    bool Configure(const std::string& path_to_config_file) {
        // сначала очищаем config_
        config_.clear();

        std::ifstream config_file_ifstream(path_to_config_file);
        if (!config_file_ifstream.is_open()) {
            return false;
        }

        json data = json::parse(config_file_ifstream);

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

    /// @brief Запускает парсер. Перед запуском необходимо настроить парсер с помощью `configure()`
    /// @param path_to_files путь до директории, содержащей файлы для обработки. Обрабатываются все файлы формата *.txt из директории
    /// @param os поток для вывода результата анализа
    void Run(std::string path_to_files, std::ostream& os = std::cout) {
        Parse(path_to_files);
        Analyze();
    }
};