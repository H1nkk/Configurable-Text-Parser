# Configurable text parser

## Запуск в Docker

Для сборки Docker-образа используйте следующую команду, находясь в корне проекта:
```bash
docker build -t configurable-parser .
```

Для запуска парсера в Docker-контейнере используйте следующую команду:
```bash
docker run -v ./test:/app/test configurable-parser ./bin/parser --config ./test/example_test/configs/example.json --data ./test/example_test/sensor_data/ --parallel
```

Флаг `-v ./test:/app/test` монтирует локальную директорию `test/` в контейнер, что позволяет парсеру видеть файлы датчиков, сгенерированные на хосте.

Подробнее про параметры запуска парсера читайте [здесь](#запуск-парсера)

## Сборка проекта
Чтобы собрать проект, используйте следующую команду, находясь в корне проекта (на системе должен быть установлен python):
```bash
python3 build.py
```

## Генерация тестовых файлов
Для генерации тестовых файлов используйте file_generator.py:
```bash
python3 .\file_generator.py <число_генерируемых_файлов> <число_датчиков_в_файле>

# Пример:
python3 .\file_generator.py 1000 4
```
Тестовые файлы генерируются в следующих директориях:
* Файл конфигурации `config.json` генерируется в директории `test/complex_test/configs/`
* Файлы, содержащие данные датчиков, генерируются в директории `test/complex_test/sensor_data/`

## Запуск парсера
После успешной сборки, в директории `bin/` будет создан исполняемый файл `parser` (или `parser.exe`, в зависимости от используемой системы)

Для запуска парсера используйте следующую команду:
```bash
./bin/parser --config <путь_до_файла_конфигурации> --data <путь_до_директории_с_файлами_датчиков> [--parallel]

# Пример:
./bin/parser --config ./test/example_test/configs/example.json --data ./test/example_test/sensor_data/ --parallel
```

Флаг `--parallel` включает параллельную обработку файлов.

Для получения информации о флагах, используйте флаг `--help` или `-h`:
```bash
# Пример:
./bin/parser -h
```

## Пример вывода
Для следующей конфигурации:
```json
{
    "sensors" : [
        { "name" : "sensor1", "rule" : "Датчик 1" },
        { "name" : "sensor2", "rule" : "Датчик 2" }
    ],

    "rules" : [
        { "name" : "state", "type" : "bool",  "rule" : "Состояние: (.*)", "true" : "включен", "false" : "выключен" },
        { "name" : "temp",  "type" : "value", "rule" : "Температура: (.*)" },
        { "name" : "speed", "type" : "speed", "rule" : "Скорость: (.*) (.*)/s" }
    ],

    "extractors" : [
        { "sensor" : "sensor1", "rules" : [ "temp", "speed" ] },
        { "sensor" : "sensor2", "rules" : [ "state", "speed" ] }
    ]
}
```

И следующих файлов:
```text
file1.txt
// Показания датчика 1
    Датчик 1: 
        Состояние: включен // комментарий, не влияет на парсинг
        Температура: 36.6
        Скорость: 765 Mbit/s

    // Показания датчика 2
    Датчик 2:
        Состояние: выключен
        Температура: 0ннн // тут опечатка
        Скорость: 1.5 Kbit/s

file2.txt
// Показания датчика 1
    Датчик 1:
        Состояние: включен
      Температура: 42
        Скорость: 1.4 Gbit/s

   // Показания датчика 2
   Датчик 2:
        Состояние: влкючен // тут опечатка
        Температура: 37
       Скорость: 0 bit/s
```

Программа выдаст следующий результат:
```
> ./bin/parser

Parsing error at file test/example_test/sensor_data/file1.txt, line 10: Couldn't parse this line
Parsing error at file test/example_test/sensor_data/file2.txt, line 9: Couldn't parse this line
sensor1:
  temp: max=42(file2.txt), min=36.6(file1.txt)
  speed: max=1.4 Gbit/s(file2.txt), min=765 Mbit/s(file1.txt)

sensor2:
  state: max=выключен(file1.txt), min=выключен(file1.txt)
  speed: max=1.5 Kbit/s(file1.txt), min=0 bit/s(file2.txt)
```

## Архитектура проекта
![](documentation/images/Project-architecture.png)