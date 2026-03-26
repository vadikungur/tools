# Сборка проекта COM Logger с использованием CMake и Qt 6.9.3 (MinGW)

## Требования
- Qt 6.9.3 с компонентом MinGW 64-bit
- CMake 3.16 или выше
- MinGW GCC (входит в состав Qt)
- VSCode (опционально, для удобной разработки)

## Сборка из командной строки

### 1. Откройте Qt Command Prompt для MinGW
Или выполните настройку переменных среды вручную:
```cmd
set PATH=C:\Qt\6.9.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%
```

### 2. Создайте директорию сборки и выполните конфигурацию
```cmd
mkdir build\debug
cd build\debug
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/Qt/6.9.3/mingw_64 ../..
```

### 3. Соберите проект
```cmd
cmake --build . --config Debug
```

Или используйте mingw32-make напрямую:
```cmd
mingw32-make
```

### 4. Запустите приложение
```cmd
bin\comlogger.exe
```

## Сборка в VSCode

### 1. Откройте папку проекта в VSCode
VSCode автоматически обнаружит CMakeLists.txt и предложит настроить проект.

### 2. Настройте CMake
Убедитесь, что в настройках VSCode указан правильный путь к Qt:
- `cmake.configureArgs`: `-DCMAKE_PREFIX_PATH=C:/Qt/6.9.3/mingw_64`

### 3. Сборка
- Нажмите `Ctrl+Shift+B` для запуска задачи сборки по умолчанию (Debug)
- Или выберите задачу через `Terminal → Run Task...`:
  - "Build All Debug" — сборка отладочной версии
  - "Build All Release" — сборка релизной версии

### 4. Отладка
- Нажмите `F5` для запуска отладки
- Выберите конфигурацию "Windows Qt6 Debug (MinGW)"

## Сборка Release-версии

```cmd
mkdir build\release
cd build\release
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.9.3/mingw_64 ../..
cmake --build . --config Release
```

## Структура проекта

```
comlogger/
├── CMakeLists.txt          # Файл конфигурации CMake
├── main.cpp                # Точка входа
├── mainwindow.h            # Заголовок основного окна
├── mainwindow.cpp          # Реализация основного окна
├── comlogger.pro           # Файл проекта qmake (альтернативная сборка)
├── README.md               # Документация
├── README_RU.md            # Документация на русском
├── .vscode/                # Настройки VSCode
│   ├── launch.json         # Конфигурация отладки
│   ├── tasks.json          # Задачи сборки
│   └── settings.json       # Настройки редактора
└── build/                  # Директория сборки (создаётся автоматически)
    ├── debug/              # Отладочная сборка
    │   └── bin/
    │       └── comlogger.exe
    └── release/            # Релизная сборка
        └── bin/
            └── comlogger.exe
```

## Примечания

1. **Путь к Qt**: Если Qt установлен в другую директорию, измените `CMAKE_PREFIX_PATH` соответственно.

2. **Версия MinGW**: Путь к отладчику в `launch.json` может отличаться в зависимости от версии MinGW. Проверьте актуальный путь в директории `C:\Qt\Tools\mingwXXXX_64\bin\`.

3. **CMake Generator**: Для MSVC используйте генератор "Visual Studio 17 2022" вместо "MinGW Makefiles".

4. **Переменные среды**: Убедитесь, что пути к Qt и MinGW добавлены в переменную среды PATH, либо используйте Qt Command Prompt.

## Устранение проблем

### Ошибка: "Qt6::SerialPort not found"
Убедитесь, что компонент Qt SerialPort установлен через Maintenance Tool.

### Ошибка: "mingw32-make: command not found"
Добавьте путь к MinGW в PATH:
```cmd
set PATH=C:\Qt\Tools\mingw1310_64\bin;%PATH%
```

### Ошибка: "CMake not found"
Установите CMake с официального сайта https://cmake.org/download/ или через Chocolatey:
```cmd
choco install cmake
```
