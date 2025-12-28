file(GLOB CORE_SOURCES "src/*.cpp")

add_library(novalink_core STATIC ${CORE_SOURCES})
target_include_directories(novalink_core PUBLIC include)
target_link_libraries(novalink_core PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        $<IF:$<TARGET_EXISTS:SQLite3::SQLite3>,SQLite3::SQLite3,${SQLite3_LIBRARIES}>
)
target_include_directories(novalink_core PUBLIC
        include
        ${SQLite3_INCLUDE_DIRS}
)