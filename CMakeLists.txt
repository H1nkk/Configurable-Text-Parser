# Минимальная версия CMake
cmake_minimum_required(VERSION 3.28)

# Название проекта
project(ConfigurableTextParser VERSION 1.0.0 LANGUAGES CXX)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# Включаем экспериментальную поддержку модулей
# set(CMAKE_CXX_SCAN_FOR_MODULES ON)

# Устанавливаем пути для включаемых файлов (аналог -I)
set(INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)

set(EXECUTABLE_NAME "parser")

# Создаем исполняемый файл из исходников
add_executable(${EXECUTABLE_NAME}
    src/source.cpp
    src/configurable_parser.cpp
)

set(CMAKE_CXX_MODULE_STD ON)

# Добавляем директории для поиска заголовочных файлов
target_include_directories(${EXECUTABLE_NAME} 
    PRIVATE 
        ${INCLUDES_DIR}
)

# Находим пакет OpenMP
find_package(OpenMP REQUIRED)

# Добавляем OpenMP к целевому проекту
if(OpenMP_CXX_FOUND)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE USE_OPENMP)
    message(STATUS "OpenMP found and enabled")
    
    # Дополнительные опции для OpenMP
    if(MSVC)
        # Для MSVC
        target_compile_options(${EXECUTABLE_NAME} PRIVATE /openmp)
    else()
        # Для GCC/Clang
        target_compile_options(${EXECUTABLE_NAME} PRIVATE -fopenmp)
    endif()
else()
    message(WARNING "OpenMP not found, parallel mode will be disabled")
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE USE_OPENMP=0)
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

# Выводим информацию о путях (полезно для отладки)
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Executable target: ${EXECUTABLE_NAME}")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Includes dir: ${INCLUDES_DIR}")