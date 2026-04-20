#pragma once
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <iostream>
#include <fstream>
#include <json.hpp>
#include <filesystem>
#include <regex>
#include <sstream>
using json = nlohmann::json;

class Parser {
private:


// ============== Структуры класса ==============

    struct SpeedValue {
        float value;
        std::string unit;
        float value_in_mbit;
        SpeedValue(float v, const std::string& u);
    };

    struct Rule {
private:
        std::unordered_map<std::string, std::string> properties_;
public:
        std::string& operator[](const std::string& propetry_name);

        const std::string& operator[](const std::string& property_name) const;

        friend std::ostream& operator<<(std::ostream& os, const Rule& rule) {
            for (auto& [key, value] : rule.properties_) {
                os << "    " << key << ": " << value << '\n'; 
            }
            return os;
        }
    };

    struct SensorData {
        std::string infile_sensor_name_;
        std::string output_sensor_name_;
        std::unordered_map<std::string, std::variant<bool, float, SpeedValue>> property_to_value_;

        void AddPropertyValue(const std::string& rule_type, const std::string& rule_name, const std::smatch& match, Rule& rule);
        

        void Dump(std::ostream& os = std::cout, const std::string& indent = "") const;
    };

    struct FileData {
        std::string file_name_;
        std::vector<SensorData> sensors_;

        FileData(std::string&& file_name = "", std::vector<SensorData>&& sensors = {}) : file_name_(file_name), sensors_(sensors) {} 

        void Dump(std::ostream& os = std::cout,  const std::string& indent = "") const;    
    };
    
    struct Config {
        std::unordered_map<std::string, std::string> sensors_rule_to_name_;
        std::unordered_map<std::string, Rule> rules_;
        std::unordered_map<std::string, std::vector<std::string>> extractors_;

        void Clear();

        void Dump(std::ostream& os = std::cout,  const std::string& indent = "") const;
    };
    
    struct SensorPropertyValue {
        std::variant<bool, float, SpeedValue> actual_value_;

        SensorPropertyValue() : actual_value_(false) {}
    };

    struct SensorMaxMinValues {
        std::string max_containing_file_;
        SensorPropertyValue max_;
        std::string min_containing_file_;
        SensorPropertyValue min_;
        bool initialized_ = false;

        SensorMaxMinValues() = default;
    };

    struct SensorAnalysisResult {
        std::string infile_sensor_name_;
        std::string output_sensor_name_;

        std::unordered_map<std::string, SensorMaxMinValues>  property_to_max_min_;
    };


// ============== Поля класса ==============

    Config config_;
    std::vector<FileData> file_data_list_;
    const std::vector<std::string> data_rates_list_ {
        "bit", "Kbit", "Mbit", "Gbit", "Tbit", "Pbit"
    };


// ============== Методы класса ==============

    double GetNumericValue(const std::variant<bool, float, SpeedValue>& value) const;

    std::string ValueToString(const std::variant<bool, float, SpeedValue>& value, 
        const std::string& true_name = "true", const std::string& false_name = "false") const;

    FileData ParseFile(const std::filesystem::path& path_to_file);
    void ParseParallel(const std::string& path_to_files);
    void ParseNonParallel(const std::string& path_to_files);
    
    void AnalyzeFile(const FileData& file_data, std::unordered_map<std::string, SensorAnalysisResult>& sensor_to_analysis_result);
    void Analyze(std::ostream& os = std::cout);

public:

    /// @brief задаёт конфигурацию парсера на основе файла конфигурации
    /// @param path_to_config_file путь до файла конфигурации
    /// @return false если открыть файл не удалось, иначе true
    bool Configure(const std::string& path_to_config_file);

    /// @brief Запускает парсер. Перед запуском необходимо настроить парсер с помощью `configure()`
    /// @param path_to_files путь до директории, содержащей файлы для обработки. Обрабатываются все файлы формата *.txt из директории
    /// @param os поток для вывода результата анализа
    /// @param parallel_mode `true` - использовать параллельную версию парсинга, `false` - использовать непараллельную версию парсинга
    void Run(std::string path_to_files, std::ostream& os = std::cout, bool parallel_mode = false);
};