cmake_minimum_required(VERSION 3.31)

project(AntivirusService)

# Указываем стандарт C++
set(CMAKE_CXX_STANDARD 20)

# Подключаем статическую библиотеку cURL
add_library(curl STATIC IMPORTED)
set_target_properties(curl PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/curl-8.11.0/builds/libcurl-vc-x86-release-static-ipv6-sspi-schannel/lib/libcurl_a.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/curl-8.11.0/builds/libcurl-vc-x86-release-static-ipv6-sspi-schannel/include"
)

# Объявляем пути до исходных файлов
set(SOURCE_FILES
    "src/Antivirus_Service.cpp"
    "src/Logger.cpp"
    "src/RequestHandler.cpp"
)

# Добавляем исполняемый файл и исходные файлы
add_executable(AntivirusService ${SOURCE_FILES})

# Указываем директорию с заголовочными файлами
target_include_directories(AntivirusService PRIVATE 
    "${CMAKE_SOURCE_DIR}/lib/curl-8.11.0/builds/libcurl-vc-x86-release-static-ipv6-sspi-schannel/include"
)

# Линкуем библиотеки: libcurl + системные библиотеки
target_link_libraries(AntivirusService 
    curl                  # Библиотека cURL
    Ws2_32               # Библиотека Windows Sockets
    Crypt32              # Библиотека шифрования
    Wldap32              # LDAP API
    Normaliz             # Unicode Normalization API
)