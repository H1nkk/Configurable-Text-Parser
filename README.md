# Configurable-Text-Parser

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

## Архитектура проекта
![](documentation/images/Project-architecture.png)