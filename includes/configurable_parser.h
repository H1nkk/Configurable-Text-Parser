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

using json = nlohmann::json;

// TODO: глобально:
// дописать Analyze()
// распараллелить
// разобраться с cmake
// написать генератор анализируемых файлов
// гугл тесты
// сделать docker

// TODO: добавить всякие проверки в Parser::Configure
// TODO: заменить map на unordered_map где нужно
// TODO: сделать проверку что нет rule с одинаковым именем в json-файле

class Parser {
private:
    enum types { // TODO удалить?
        kBool = 0,
        kFloat = 1,
        kUnsignedLL = 2
    };
    
// *** Структуры класса ***
    struct SpeedValue {
        float value;
        std::string unit;
        float value_in_mbit;
        SpeedValue(float v, const std::string& u) : value(v), unit(u) {
        if (unit == "Kbit") value_in_mbit = value / 1000.0f;
        else if (unit == "Mbit") value_in_mbit = value;
        else if (unit == "Gbit") value_in_mbit = value * 1000.0f;
        else if (unit == "Tbit") value_in_mbit = value * 1000000.0f;
        else if (unit == "Pbit") value_in_mbit = value * 1000000000.0f;
        else if (unit == "bit") value_in_mbit = value / 1000000.0f;
        else value_in_mbit = value;
    }
    };

    struct Rule {
        std::map<std::string, std::string> properties_;

        // TODO посмотреть: раньше это было в public, а properties_ - в private. странное решение 
        std::string& operator[](const std::string& propetry_name) {
            return properties_[propetry_name];
        }


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
        std::map<std::string, std::variant<bool, float, SpeedValue>> property_to_value_;

        void AddPropertyValue(const std::string& rule_type, const std::string& rule_name, const std::smatch& match, Rule& rule) {
            if (rule_type == "bool") {
                if (match[1] == rule["true"]) {
                    property_to_value_[rule_name] = true;
                } else { 
                    // тут (match[1] == rule["false"])
                    property_to_value_[rule_name] = false;
                }
            } else if (rule_type == "value") {
                property_to_value_[rule_name] = std::stof(match[1]);
            } else if (rule_type == "speed") {
                if (match.size() >= 3) {
                    try {
                        float value = std::stof(match[1].str());
                        std::string unit = match[2].str();
                        
                        // создаем SpeedValue и кладём в variant
                        SpeedValue speed(value, unit);
                        property_to_value_[rule_name] = speed;
                        
                        std::cout << "  Скорость: " << speed.value << " " << speed.unit 
                                << " (" << speed.value_in_mbit << " Mbit/s)" << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "Ошибка парсинга скорости: " << e.what() << std::endl;
                    }
                }
            }
        }

        void Dump(std::ostream& os = std::cout, const std::string& indent = "") const {
            os << indent << "infile_sensor_name_: " << infile_sensor_name_ << '\n';
            os << indent << "output_sensor_name_: " << output_sensor_name_ << '\n';
            os << indent << "property_to_value_:\n";
            
            for (const auto& [key, value] : property_to_value_) {
                os << indent << "  " << key << ": ";
                
                if (std::holds_alternative<bool>(value)) {
                    bool val = std::get<bool>(value);
                    os << (val ? "true" : "false");
                }
                else if (std::holds_alternative<float>(value)) {
                    float val = std::get<float>(value);
                    os << val;
                }
                else if (std::holds_alternative<SpeedValue>(value)) {
                    SpeedValue val = std::get<SpeedValue>(value);
                    os << val.value << " " << val.unit << "/s";
                }
                
                os << '\n';
            }
        }
    };

    struct FileData {
        std::string file_name_;
        std::vector<SensorData> sensors_;

        FileData(std::string&& file_name = "", std::vector<SensorData>&& sensors = {}) : file_name_(file_name), sensors_(sensors) {} 

        void Dump(std::ostream& os = std::cout,  const std::string& indent = "") const {
            os << indent << "file_name_: " << file_name_ << '\n';
            os << indent << "sensors_:\n";
            
            if (sensors_.empty()) {
                os << indent << "(empty)\n";
            } else {
                for (size_t i = 0; i < sensors_.size(); ++i) {
                    os << indent << "sensor[" << i << "]:\n";
                    sensors_[i].Dump(os, indent + "  ");
                }
            }
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

        void Clear() {
            sensors_rule_to_name_.clear();
            rules_.clear();
            extractors_.clear();
        }

        void Dump(std::ostream& os,  const std::string& indent = "") const {
            os << indent << "  sensors_rule_to_name_:\n";
            for (auto& [key, value] : sensors_rule_to_name_) {
                os << indent << key << ": " << value << '\n';
            }

            os << indent << "  rules_:\n";
            for (auto& [key, rule] : rules_) {
                os << indent << key << ":\n";
                os << indent << "  " << rule;
            }

            os << "extractors_:\n";
            for (auto& extractor : extractors_) {
                
                os << indent << "  " << extractor.name_ << ":\n";
                for (auto& rule_to_extract : extractor.rules_to_extract_) {
                    os << indent << "    " << rule_to_extract << "\n";
                }
            }
        }
    };
    
    struct SensorPropertyValue {
        std::string infile_property_value_;
        std::variant<bool, float, SpeedValue> actual_value_;

        SensorPropertyValue() : actual_value_(false) {}
    };

    struct SensorMaxMinValues {
        std::string max_containing_file_;
        SensorPropertyValue max_;
        std::string min_containing_file_;
        SensorPropertyValue min_;

        SensorMaxMinValues() = default;
    };

    struct SensorAnalysisResult {
        std::string infile_sensor_name_;
        std::string output_sensor_name_;

        std::map<std::string, SensorMaxMinValues>  property_to_max_min_;
    };

// *** Поля класса ***
    Config config_;
    std::vector<FileData> file_data_list_;
    const std::vector<std::string> data_rates_list_ {
        "bit", "Kbit", "Mbit", "Gbit", "Tbit", "Pbit"
    };

// *** Методы класса ***
    template<typename T>
    static bool IsGreater(const T& a, const T& b) {
        return a > b;
    }
    
    // Специализация для SpeedValue
    static bool IsGreater(const SpeedValue& a, const SpeedValue& b) {
        return a.value_in_mbit > b.value_in_mbit;
    }
    
    // Специализация для bool (false < true)
    static bool IsGreater(bool a, bool b) {
        return a && !b;
    }

    double GetNumericValue(const std::variant<bool, float, SpeedValue>& value) const {
        if (std::holds_alternative<float>(value)) {
            return std::get<float>(value);
        } else if (std::holds_alternative<SpeedValue>(value)) {
            return std::get<SpeedValue>(value).value_in_mbit;
        } else if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? 1.0 : 0.0;
        }
        return 0.0;
    }

    std::string ValueToString(const std::variant<bool, float, SpeedValue>& value) const {
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? "true" : "false";
        } 
        else if (std::holds_alternative<float>(value)) {
            return std::to_string(std::get<float>(value));
        }
        else if (std::holds_alternative<SpeedValue>(value)) {
            const auto& sv = std::get<SpeedValue>(value);
            return std::to_string(sv.value) + " " + sv.unit;
        }
        return "unknown";
    }


    FileData ParseFile(const std::filesystem::path& path_to_file) {
        std::string file_name = path_to_file.filename().string();
        std::ifstream data_file_ifstream(path_to_file);
        if (!data_file_ifstream.is_open()) {
            throw "Couldn't open " + path_to_file.string() + " file."; // TODO обернуть в красивый тип исключения
        }

        std::string line = "";
        std::string current_infile_sensor_name = "";
        bool is_sensor_seen = false;
        std::vector<SensorData> sensors;

        while (std::getline(data_file_ifstream, line)) {
            // проверяем на наличие комментариев
            size_t commentary_pos = line.find("//");
            if (commentary_pos != std::string::npos) {
                // комментарий есть, обрезаем строку
                line = line.substr(0, commentary_pos);
            }

            // убираем ведущие и последние незначащие пробелы
            try {
                if (line.find_first_not_of(" \t\n\r") != std::string::npos) {
                    line = line.substr(line.find_first_not_of(" \t\n\r"), line.find_last_not_of(" \t\n\r") - line.find_first_not_of(" \t\n\r") + 1);
                } 
                else {
                    continue;
                }
            }
            catch (std::exception &e) {
                // TODO если мы здесь, значит line == "". надо делать просто скип
                std::cout << e.what() << '\n';
                continue;
            }


            // проверяем, не начался ли другой датчик
            if (config_.sensors_rule_to_name_.find(line.substr(0, line.size() - 1)) != config_.sensors_rule_to_name_.end()) {
                // line - имя датчика
                std::string sensor_name = line;

                if (!sensor_name.empty() && sensor_name.back() == ':') {
                    sensor_name.pop_back();
                }

                current_infile_sensor_name = sensor_name;
                is_sensor_seen = false;
                continue;
            }

            std::cout << "line: " << line << " datchik: " << current_infile_sensor_name << '\n';


            // проверяем на соответствие 
            std::string regex_pattern;
            
            
            // проходимся по всем правилам
            for (auto& [rule_name, rule] : config_.rules_) {
                regex_pattern = rule["rule"];
                std::regex pattern(regex_pattern);
                std::smatch match;

                if (std::regex_match(line, match, pattern)) {
                    // std::cout << "ETO MATCH " << match[1] << '\n';

                    if (is_sensor_seen) {
                        auto& sensor_data = sensors[sensors.size() - 1];
                        std::string rule_type = rule["type"];
                        sensor_data.AddPropertyValue(rule_type, rule_name, match, rule);
                    }
                    else {
                        SensorData sensor_data;
                        sensor_data.infile_sensor_name_ = current_infile_sensor_name;
                        sensor_data.output_sensor_name_ = config_.sensors_rule_to_name_[current_infile_sensor_name];

                        std::string rule_type = rule["type"];
                        sensor_data.AddPropertyValue(rule_type, rule_name, match, rule);
                        sensors.push_back(sensor_data);
                    }
                    is_sensor_seen = true;
                    break;
                }
                
            }
        }

        for (auto& e: sensors) {
            e.Dump(std::cout);
        }
        
        FileData result(std::move(file_name), std::move(sensors));

        return result;
    }
    void Parse(const std::string& path_to_files) { // заполняет files_data
        // тут можно сделать проверку, что есть хотя бы один файл для парсинга
        // ещё сюда можно добавить параллельности 
        // TODO сделать чтобы параллельность переключалась флагом --parallel
        std::filesystem::path test{path_to_files};
        for (auto const& dir_entry : std::filesystem::directory_iterator{test}) {
            if (dir_entry.is_regular_file()) {
                std::filesystem::path file_path = dir_entry.path();
                if (file_path.extension() == ".txt") {
                    file_data_list_.push_back(ParseFile(file_path));
                }
            }
        }

        std::cout<<"###\n";
        for (auto x : file_data_list_) {
            x.Dump();
        }
    }
    
    void AnalyzeFile(const FileData& file_data, 
                 std::map<std::string, SensorAnalysisResult>& sensor_to_analysis_result) {
        const std::string& current_file = file_data.file_name_;
        
        for (const auto& sensor_data : file_data.sensors_) {
            const std::string& sensor_name = sensor_data.output_sensor_name_;
            
            auto& analysis_result = sensor_to_analysis_result[sensor_name];
            analysis_result.output_sensor_name_ = sensor_name;
            analysis_result.infile_sensor_name_ = sensor_data.infile_sensor_name_;
            
            for (const auto& [property_name, property_value] : sensor_data.property_to_value_) {
                auto& max_min = analysis_result.property_to_max_min_[property_name];
                
                // Обновляем максимум
                if (max_min.max_.actual_value_.index() == 0 || 
                    GetNumericValue(property_value) > GetNumericValue(max_min.max_.actual_value_)) {
                    max_min.max_.actual_value_ = property_value;
                    max_min.max_containing_file_ = current_file;
                    max_min.max_.infile_property_value_ = ValueToString(property_value);
                }
                
                // Обновляем минимум
                if (max_min.min_.actual_value_.index() == 0 || 
                    GetNumericValue(property_value) < GetNumericValue(max_min.min_.actual_value_)) {
                    max_min.min_.actual_value_ = property_value;
                    max_min.min_containing_file_ = current_file;
                    max_min.min_.infile_property_value_ = ValueToString(property_value);
                }
            }
        }
    }
    
    void Analyze(std::ostream& os = std::cout) {
        std::map<std::string, SensorAnalysisResult> sensor_to_analysis_result;
        for (const auto& file_data : file_data_list_) {
            AnalyzeFile(file_data, sensor_to_analysis_result);
        }
    }

public:

    /// @brief configures parser based on config file
    /// @param path_to_config_file путь до файла конфигурации
    /// @return false если открыть файл не удалось, иначе true
    bool Configure(const std::string& path_to_config_file) {
        // сначала очищаем config_
        config_.Clear();

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

            // приводим properties["rule"] к правильному для regexpr формату
            std::string rule_type = properties["type"];
            if (rule_type == "bool") {
                std::string initial_string = properties["rule"];
                size_t template_marker_pos = initial_string.find("(.*)");
                std::string regexpr_pattern = initial_string.substr(0, template_marker_pos);
                
                regexpr_pattern += "(";
                regexpr_pattern += properties["true"];
                regexpr_pattern += "|";
                regexpr_pattern += properties["false"];
                regexpr_pattern += ")";

                regexpr_pattern += initial_string.substr(template_marker_pos + 4); // 4 это длина строки "(.*)"
                std::cout << "получился regexpr: " << regexpr_pattern << '\n';
                properties["rule"] = regexpr_pattern;
            }
            else if (rule_type == "value") {
                std::string initial_string = properties["rule"];
                size_t template_marker_pos = initial_string.find("(.*)");
                std::string regexpr_pattern = initial_string.substr(0, template_marker_pos);
                
                regexpr_pattern += R"(([+-]?\d+(?:\.\d+)?))"; // TODO проверить, обязательно ли вообще R-строки юзать 

                regexpr_pattern += initial_string.substr(template_marker_pos + 4);
                std::cout << "получился regexpr: " << regexpr_pattern << '\n';
                properties["rule"] = regexpr_pattern;
            }
            else if (rule_type == "speed") {
                std::string initial_string = properties["rule"];
                size_t template_marker_pos = initial_string.find("(.*)");
                std::string regexpr_pattern = initial_string.substr(0, template_marker_pos);

                regexpr_pattern += R"(([+-]?\d+(?:\.\d+)?))";

                size_t second_template_marker_pos = initial_string.find("(.*)", template_marker_pos + 4);
                regexpr_pattern += initial_string.substr(template_marker_pos + 4, second_template_marker_pos - (template_marker_pos + 4));
                regexpr_pattern += "(";
                regexpr_pattern += data_rates_list_[0];

                for (int i = 1; i < data_rates_list_.size(); i++) {
                    regexpr_pattern += "|";
                    regexpr_pattern += data_rates_list_[i];
                }
                regexpr_pattern += ")";

                regexpr_pattern += initial_string.substr(second_template_marker_pos + 4);
                std::cout << "получился regexpr: " << regexpr_pattern << '\n';
                properties["rule"] = regexpr_pattern;
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
        config_.Dump(std::cout);
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