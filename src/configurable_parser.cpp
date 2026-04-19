#include "configurable_parser.h"


// Parser::SpeedValue методы

Parser::SpeedValue::SpeedValue(float v, const std::string& u) : value(v), unit(u) {
    if (unit == "Kbit") value_in_mbit = value / 1000.0f;
    else if (unit == "Mbit") value_in_mbit = value;
    else if (unit == "Gbit") value_in_mbit = value * 1000.0f;
    else if (unit == "Tbit") value_in_mbit = value * 1000000.0f;
    else if (unit == "Pbit") value_in_mbit = value * 1000000000.0f;
    else if (unit == "bit") value_in_mbit = value / 1000000.0f;
    else value_in_mbit = value;
}


// Parser::Rule методы

std::string& Parser::Rule::operator[](const std::string& propetry_name) {
    return properties_[propetry_name];
}

const std::string& Parser::Rule::operator[](const std::string& property_name) const {
    static const std::string empty;
    auto it = properties_.find(property_name);
    if (it != properties_.end()) {
        return it->second;
    }
    return empty;
}


// Parser::SensorData методы

void Parser::SensorData::AddPropertyValue(const std::string& rule_type, const std::string& rule_name, const std::smatch& match, Rule& rule) {
    if (rule_type == "bool") {
        if (match[1] == rule["true"]) {
            property_to_value_[rule_name] = true;
        } else { 
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
            } catch (const std::exception& e) {
                std::cerr << "Speed parsing error: " << e.what() << std::endl;
            }
        }
    }
}

void Parser::SensorData::Dump(std::ostream& os, const std::string& indent) const {
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


// Parser::FileData методы

void Parser::FileData::Dump(std::ostream& os,  const std::string& indent) const {
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


// Parser::Config методы

void Parser::Config::Clear() {
    sensors_rule_to_name_.clear();
    rules_.clear();
    extractors_.clear();
}

void Parser::Config::Dump(std::ostream& os,  const std::string& indent) const {
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
    for (auto& [extractor_name, extractor_rules_to_extract_] : extractors_) {
        
        os << indent << "  " << extractor_name << ":\n";
        for (auto& rule_to_extract : extractor_rules_to_extract_) {
            os << indent << "    " << rule_to_extract << "\n";
        }
    }
}


// Parser методы

double Parser::GetNumericValue(const std::variant<bool, float, SpeedValue>& value) const {
    if (std::holds_alternative<float>(value)) {
        return std::get<float>(value);
    } else if (std::holds_alternative<SpeedValue>(value)) {
        return std::get<SpeedValue>(value).value_in_mbit;
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? 1.0 : 0.0;
    }
    return 0.0;
}

std::string Parser::ValueToString(const std::variant<bool, float, SpeedValue>& value, 
    const std::string& true_name, const std::string& false_name) const {
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? true_name : false_name;
    } 
    else if (std::holds_alternative<float>(value)) {
        std::ostringstream oss;
        oss << std::get<float>(value);
        return oss.str();
    }
    else if (std::holds_alternative<SpeedValue>(value)) {
        const auto& sv = std::get<SpeedValue>(value);
        std::ostringstream oss;
        oss << sv.value << " " << sv.unit << "/s";
        return oss.str();
    }
    return "unknown";
}

Parser::FileData Parser::ParseFile(const std::filesystem::path& path_to_file) {
    std::string file_name = path_to_file.filename().string();
    std::ifstream data_file_ifstream(path_to_file);
    if (!data_file_ifstream.is_open()) {
        throw std::runtime_error("Couldn't open " + path_to_file.string() + " file.");
    }

    std::string line = "";
    std::string current_infile_sensor_name = "";
    bool is_sensor_seen = false;
    std::vector<SensorData> sensors;

    std::vector<std::string> errors;

    unsigned int line_number = 0;
    while (std::getline(data_file_ifstream, line)) {
        ++line_number;
        // проверяем на наличие комментариев
        size_t commentary_pos = line.find("//");
        if (commentary_pos != std::string::npos) {
            // комментарий есть, обрезаем строку
            line = line.substr(0, commentary_pos);
        }

        // убираем ведущие и последние незначащие пробелы
        try {
            if (line.find_first_not_of(" \t\n\r") != std::string::npos) {
                line = line.substr(
                    line.find_first_not_of(" \t\n\r"),
                    line.find_last_not_of(" \t\n\r") - line.find_first_not_of(" \t\n\r") + 1
                );
            } 
            else {
                continue;
            }
        }
        catch (std::exception &e) {
            std::cout << e.what() << '\n';
            continue;
        }

        // проверяем, не начался ли другой датчик
        if (config_.sensors_rule_to_name_.find(line.substr(0, line.size() - 1)) != config_.sensors_rule_to_name_.end()) {
            std::string sensor_name = line;

            if (!sensor_name.empty() && sensor_name.back() == ':') {
                sensor_name.pop_back();
            }

            current_infile_sensor_name = sensor_name;
            is_sensor_seen = false;
            continue;
        }

        // проверяем на соответствие 
        std::string regex_pattern;
        bool matched = false;
        
        // проходимся по всем правилам
        for (auto& [rule_name, rule] : config_.rules_) {
            regex_pattern = rule["rule"];
            std::regex pattern(regex_pattern);
            std::smatch match;

            if (std::regex_match(line, match, pattern)) {
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
                matched = true;
                break;
            }
        }

        if (!matched) {
            errors.push_back("Parsing error at file " + path_to_file.string() 
                        + ", line " + std::to_string(line_number) 
                        + ": Couldn't parse this line");
        }

    }

    for (const auto& err : errors) {
        std::cout << err << '\n';
    }
    
    FileData result(std::move(file_name), std::move(sensors));
    return result;
}

void Parser::ParseParallel(const std::string& path_to_files) { // заполняет files_data
    std::filesystem::path test{path_to_files};
    std::vector<std::filesystem::path> files_to_process_list;
    
    for (auto const& dir_entry : std::filesystem::directory_iterator{test}) {
        if (dir_entry.is_regular_file()) {
            std::filesystem::path file_path = dir_entry.path();
            if (file_path.extension() == ".txt") {
                files_to_process_list.push_back(file_path);
            }
        }
    }
    file_data_list_.resize(files_to_process_list.size());

    size_t files_to_process_count = files_to_process_list.size();

#pragma omp parallel for
    for (int i = 0; i < files_to_process_count; i++) {
        file_data_list_[i] = ParseFile(files_to_process_list[i]);
    }
}

void Parser::ParseNonParallel(const std::string& path_to_files) { // заполняет files_data
    std::filesystem::path test{path_to_files};
    for (auto const& dir_entry : std::filesystem::directory_iterator{test}) {
        if (dir_entry.is_regular_file()) {
            std::filesystem::path file_path = dir_entry.path();
            if (file_path.extension() == ".txt") {
                file_data_list_.push_back(ParseFile(file_path));
            }
        }
    }
}

void Parser::AnalyzeFile(const FileData& file_data, std::unordered_map<std::string, SensorAnalysisResult>& sensor_to_analysis_result) {
    const std::string& current_file = file_data.file_name_;
    
    for (const auto& sensor_data : file_data.sensors_) {
        const std::string& sensor_name = sensor_data.output_sensor_name_;
        
        auto& analysis_result = sensor_to_analysis_result[sensor_name];
        analysis_result.output_sensor_name_ = sensor_name;
        analysis_result.infile_sensor_name_ = sensor_data.infile_sensor_name_;
        
        for (const auto& [property_name, property_value] : sensor_data.property_to_value_) {
            auto& max_min = analysis_result.property_to_max_min_[property_name];

            if (!max_min.initialized_) {
                max_min.initialized_ = true;
                max_min.max_.actual_value_ = property_value;
                max_min.max_containing_file_ = current_file;
                max_min.min_.actual_value_ = property_value;
                max_min.min_containing_file_ = current_file;
                continue;
            }

            // сравниваем с текущим max
            if (GetNumericValue(property_value) > GetNumericValue(max_min.max_.actual_value_)) {
                max_min.max_.actual_value_ = property_value;
                max_min.max_containing_file_ = current_file;
            }

            // сравниваем с текущим min
            if (GetNumericValue(property_value) < GetNumericValue(max_min.min_.actual_value_)) {
                max_min.min_.actual_value_ = property_value;
                max_min.min_containing_file_ = current_file;
            }
        }
    }
}

void Parser::Analyze(std::ostream& os) {
    std::unordered_map<std::string, SensorAnalysisResult> sensor_to_analysis_result;

    // собираем данные со всех файлов
    for (const auto& file_data : file_data_list_) {
        AnalyzeFile(file_data, sensor_to_analysis_result);
    }

    if (sensor_to_analysis_result.empty()) {
        os << "No data found.\n";
        return;
    }

    for (const auto& [sensor_name, result] : sensor_to_analysis_result) {
        os << result.output_sensor_name_ << ":\n";
        
        // получаем список правил для текущего сенсора
        auto it = config_.extractors_.find(sensor_name);
        if (it == config_.extractors_.end()) {
            continue;
        }
        
        for (const auto& rule_name : it->second) {
            // находим правило в конфиге
            auto rule_it = config_.rules_.find(rule_name);
            if (rule_it == config_.rules_.end()) {
                continue;
            }
            
            const auto& rule = rule_it->second;
            
            // проверяем, есть ли это свойство в результате
            auto prop_it = result.property_to_max_min_.find(rule_name);
            if (prop_it == result.property_to_max_min_.end()) {
                continue;
            }
            
            const auto& max_min = prop_it->second;
            
            // меняем вывод для bool значений 
            std::string true_name = "true";
            std::string false_name = "false";

            if (rule["type"] == "bool") {
                true_name = rule["true"];
                false_name = rule["false"];
            }
            
            os << "  " << rule_name << ": ";
            
            os << "max=" << ValueToString(max_min.max_.actual_value_, true_name, false_name);
            if (!max_min.max_containing_file_.empty()) {
                os << "(" << max_min.max_containing_file_ << ")";
            }
            
            os << ", ";
            
            os << "min=" << ValueToString(max_min.min_.actual_value_, true_name, false_name);
            if (!max_min.min_containing_file_.empty()) {
                os << "(" << max_min.min_containing_file_ << ")";
            }
            
            os << "\n";
        }
        os << "\n";
    }
}

bool Parser::Configure(const std::string& path_to_config_file) {
    // сначала очищаем config_
    config_.Clear();

    std::ifstream config_file_ifstream(path_to_config_file);
    if (!config_file_ifstream.is_open()) {
        return false;
    }

    json data = json::parse(config_file_ifstream);

    for (auto& sensor : data["sensors"]) {
        config_.sensors_rule_to_name_[sensor["rule"]] = sensor["name"];
    }

    for (auto& rule : data["rules"]) {
        std::string rule_name = rule["name"];

        // проверка на дубликат
        if (config_.rules_.find(rule_name) != config_.rules_.end()) {
            throw std::runtime_error("Duplicate rule name in config: \"" + rule_name + "\"");
        }

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
            properties["rule"] = regexpr_pattern;
        }
        else if (rule_type == "value") {
            std::string initial_string = properties["rule"];
            size_t template_marker_pos = initial_string.find("(.*)");
            std::string regexpr_pattern = initial_string.substr(0, template_marker_pos);
            
            regexpr_pattern += R"(([+-]?\d+(?:\.\d+)?))";

            regexpr_pattern += initial_string.substr(template_marker_pos + 4);
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
            properties["rule"] = regexpr_pattern;
        }
    }

    for (auto& extractor : data["extractors"]) {
        std::vector<std::string> new_extractor;
        
        for (auto& rule_name : extractor["rules"]) {
            new_extractor.push_back(rule_name);
        }

        config_.extractors_[extractor["sensor"]] = (std::move(new_extractor));
    }


    return true;
}

void Parser::Run(std::string path_to_files, std::ostream& os, bool parallel_mode) {
    try {
        if (parallel_mode) {
            ParseParallel(path_to_files);
        }
        else {
            ParseNonParallel(path_to_files);
        }
        Analyze(os);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}

