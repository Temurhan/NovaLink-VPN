find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
file(GLOB_RECURSE CLIENT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(novalink_client ${CLIENT_SOURCES})
target_include_directories(novalink_client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/internal/tun
        ${CMAKE_CURRENT_SOURCE_DIR}/internal/outbound
        ${CMAKE_CURRENT_SOURCE_DIR}/discovery
        ${CMAKE_SOURCE_DIR}/core/include
        ${CURL_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(novalink_client PRIVATE
        novalink_core
        CURL::libcurl
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
        dl
)