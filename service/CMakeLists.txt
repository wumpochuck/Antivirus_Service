cmake_minimum_required(VERSION 3.30)

project(AntivirusService)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Отдельная сборка
add_subdirectory(lib/logger)
add_subdirectory(lib/curl-8.11.0)

# Исходные файлы
set(SOURCE_FILES
    "apps/antimalware_service_app/src/SvcInit.cpp"
    "apps/antimalware_service_app/src/SvcThread.cpp"
)

# Исполняемый файл
add_executable(AntivirusService ${SOURCE_FILES})

# Директории с .h файлами
target_include_directories(AntivirusService PRIVATE
    # "${CMAKE_SOURCE_DIR}/lib/curl-8.11.0/builds/libcurl-vc-x86-release-static-ipv6-sspi-schannel/include"
    "${CMAKE_SOURCE_DIR}/lib/logger/include"
    "${CMAKE_SOURCE_DIR}/lib/curl-8.11.0/include"
)

# Линковка библиотек
target_link_libraries(AntivirusService 
    Logger                # Библиотека Logger
    libcurl               # Библиотека cURL
    Ws2_32                # Библиотека Windows Sockets
    Crypt32               # Библиотека шифрования
    Wldap32               # LDAP API
    Normaliz              # Unicode Normalization API
)


# cmake ..
# cmake --build . --config Release