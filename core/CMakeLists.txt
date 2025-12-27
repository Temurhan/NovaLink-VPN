file(GLOB CORE_SOURCES "src/*.cpp")

add_library(novalink_core STATIC ${CORE_SOURCES})

# Важно: указываем, где лежат хедеры ядра
target_include_directories(novalink_core PUBLIC include)

# Линкуем системные либы, нужные ядру
target_link_libraries(novalink_core PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        # Проверяем, существует ли современный таргет, если нет — используем переменную
        $<IF:$<TARGET_EXISTS:SQLite3::SQLite3>,SQLite3::SQLite3,${SQLite3_LIBRARIES}>
)

# Добавляем пути к заголовочным файлам SQLite
target_include_directories(novalink_core PUBLIC
        include
        ${SQLite3_INCLUDE_DIRS}
)