find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

set(SERVER_SRC main.cpp ${CMAKE_SOURCE_DIR}/client/internal/tun/TunHandler.cpp)
add_executable(novalink_server ${SERVER_SRC})

target_include_directories(novalink_server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/client/internal/tun
        ${CMAKE_SOURCE_DIR}/core/include
        ${CMAKE_SOURCE_DIR}/manager
        ${OPENSSL_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
)

target_link_libraries(novalink_server PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
        CURL::libcurl
)

set_target_properties(novalink_server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)