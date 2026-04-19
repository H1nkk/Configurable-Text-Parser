import os, sys, random
import json

def write_text_to_file(file_path, sensors_count):
    properties_list = ["Состояние", "Температура", "Скорость"]
    state_variants = ["включен", "выключен"]
    units_variants = ["bit/s", "Kbit/s","Mbit/s","Gbit/s","Tbit/s","Pbit/s",]

    with open(file_path, "w", encoding="utf-8") as f:
        for sensor in range(sensors_count):
            f.write(f"Датчик {sensor + 1}: \n")
            for property in properties_list:
                f.write(f"  {property}: ")
                if property == "Состояние":
                    f.write(f"{random.choice(state_variants)}\n")
                    pass
                elif property == "Температура":
                    f.write(f"{random.uniform(20, 70)}\n")
                elif property == "Скорость":
                    f.write(f"{random.uniform(0, 999.9)} {random.choice(units_variants)}\n")
            f.write("\n")
   

def write_config_to_file(file_path, sensors_count):
    # Создаем список sensors
    sensors_list = []
    for i in range(1, sensors_count + 1):
        sensors_list.append({
            "name": f"sensor{i}",
            "rule": f"Датчик {i}"
        })
    
    # Создаем список rules (фиксированный)
    rules_list = [
        {
            "name": "state",
            "type": "bool",
            "rule": "Состояние: (.*)",
            "true": "включен",
            "false": "выключен"
        },
        {
            "name": "temp",
            "type": "value",
            "rule": "Температура: (.*)"
        },
        {
            "name": "speed",
            "type": "speed",
            "rule": "Скорость: (.*) (.*)/s"
        }
    ]
    
    # Создаем список extractors
    extractors_list = []
    for i in range(1, sensors_count + 1):
        # Чередуем правила для разных датчиков
        if i % 3 == 1:
            rules = ["temp", "speed"]
        elif i % 3 == 2:
            rules = ["state", "speed"]
        else:
            rules = ["state", "temp"]
        
        extractors_list.append({
            "sensor": f"sensor{i}",
            "rules": rules
        })
    
    # Формируем итоговый JSON
    config_data = {
        "sensors": sensors_list,
        "rules": rules_list,
        "extractors": extractors_list
    }
    
    # Записываем в файл с отступами для читаемости
    with open(file_path, "w", encoding="utf-8") as f:
        json.dump(config_data, f, indent=4, ensure_ascii=False)
    
    print(f"Конфиг записан в {file_path}")

if __name__ == "__main__":
    # Проверяем количество аргументов
    if len(sys.argv) != 3:
        print("Ошибка: укажите число файлов и число датчиков")
        print(f"Использование: py {sys.argv[0]} <число_файлов> <число_датчиков>")
        sys.exit(1)
    
    try:
        num_files = int(sys.argv[1])
        sensors_per_file = int(sys.argv[2])
        
        if num_files <= 0 or sensors_per_file <= 0:
            print("Ошибка: числа должны быть положительными")
            sys.exit(1)
        
        # Создаем папку если её нет
        os.makedirs("test/sensor_data", exist_ok=True)
        
        # Генерируем файлы
        for file_num in range(1, num_files + 1):
            file_path = f"test/complex_test/sensor_data/sensor_data_{file_num}.txt"
            write_text_to_file(file_path, sensors_per_file)
        
        print(f"\nСоздано {num_files} файлов, в каждом {sensors_per_file} датчиков")
        
        # Генерируем конфиг
        file_path = f"test/complex_test/configs/config.json"
        write_config_to_file(file_path, sensors_per_file)

    except ValueError:
        print("Ошибка: аргументы должны быть целыми числами")
        sys.exit(1)