# Минимальная версия CMake
cmake_minimum_required(VERSION 3.30)

# Название проекта
project(ConfigurableTextParser VERSION 1.0.0 LANGUAGES CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# Включаем экспериментальную поддержку модулей
# set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# Устанавливаем пути для включаемых файлов (аналог -I)
set(INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)

# Создаем исполняемый файл из исходников
add_executable(${PROJECT_NAME} 
    src/source.cpp
)

set(CMAKE_CXX_MODULE_STD ON)

# Добавляем директории для поиска заголовочных файлов
target_include_directories(${PROJECT_NAME} 
    PRIVATE 
        ${INCLUDES_DIR}
)

# Опционально: настройки для разных конфигураций сборки
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

# Опционально: включаем предупреждения компилятора
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Выводим информацию о путях (полезно для отладки)
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Includes dir: ${INCLUDES_DIR}")