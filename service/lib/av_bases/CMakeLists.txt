cmake_minimum_required(VERSION 3.30)

project(av_bases_lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
    "src/loader.cpp"
)

add_library(av_bases STATIC ${SOURCE_FILES})

target_include_directories(av_bases PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}../curl-8.11.0/include"
)

target_link_libraries(av_bases 
    libcurl               # Библиотека cURL
    Ws2_32                # Библиотека Windows Sockets
    Crypt32               # Библиотека шифрования
    Wldap32               # LDAP API
    Normaliz              # Unicode Normalization API
)

