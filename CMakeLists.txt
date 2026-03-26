cmake_minimum_required(VERSION 3.16)

project(comlogger VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Находим пакет Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

# Исходные файлы проекта
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

# Создаём исполняемый файл
add_executable(comlogger ${PROJECT_SOURCES})

# Подключаем библиотеки Qt
target_link_libraries(comlogger PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
)

# Для Windows: устанавливаем имя выходного файла без префикса и добавляем .exe
if(WIN32)
    set_target_properties(comlogger PROPERTIES
        WIN32_EXECUTABLE TRUE
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
else()
    set_target_properties(comlogger PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()

# Копирование настроек VSCode (если нужно)
# Файлы .vscode уже есть в проекте, CMake их не трогает
