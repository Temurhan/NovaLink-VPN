# 1. Поиск зависимостей
find_package(OpenSSL REQUIRED)

# 2. Исходники сервера
# Обрати внимание: мы добавляем TunHandler.cpp из папки клиента,
# чтобы не дублировать код!
set(SERVER_SRC
        main.cpp
        ${CMAKE_SOURCE_DIR}/client/internal/tun/TunHandler.cpp
)

# 3. Исполняемый файл
add_executable(novalink_server ${SERVER_SRC})

# 4. Пути к заголовкам
target_include_directories(novalink_server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/client/internal/tun   # Чтобы сервер видел TunHandler.hpp
        ${CMAKE_SOURCE_DIR}/core/include
        ${CMAKE_SOURCE_DIR}/manager
        ${OPENSSL_INCLUDE_DIR}
)

# 5. Линковка библиотек
target_link_libraries(novalink_server PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
)

# 6. Настройка вывода (чтобы сервер лежал рядом с клиентом)
set_target_properties(novalink_server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)