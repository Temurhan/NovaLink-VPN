# 1. Сначала находим пакеты в системе
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# 2. Собираем все исходники (теперь CMake видит все твои .cpp файлы)
file(GLOB_RECURSE CLIENT_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# 3. Создаем исполняемый файл
add_executable(novalink_client ${CLIENT_SOURCES})

# 4. Указываем пути к заголовкам
target_include_directories(novalink_client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/internal/tun
        ${CMAKE_CURRENT_SOURCE_DIR}/internal/outbound
        ${CMAKE_CURRENT_SOURCE_DIR}/discovery
        ${CMAKE_SOURCE_DIR}/core/include
        ${CURL_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
)

# 5. ЛИНКОВКА (Самый важный шаг против "undefined reference")
target_link_libraries(novalink_client PRIVATE
        novalink_core
        CURL::libcurl
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread  # Добавляем для безопасности потоков
        dl       # Динамическая линковка
)