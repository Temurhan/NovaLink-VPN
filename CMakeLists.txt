cmake_minimum_required(VERSION 3.20)
project(NovaLink-VPN LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройка путей для бинарных файлов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# Поиск зависимостей
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

# Добавляем поддиректории
add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(manager-api)
include_directories(core/include)