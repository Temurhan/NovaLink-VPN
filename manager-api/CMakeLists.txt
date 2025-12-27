add_executable(novalink_manager main.cpp)

# Менеджеру тоже может понадобиться ядро для работы с БД
target_link_libraries(novalink_manager PRIVATE novalink_core)