cmake_minimum_required(VERSION 3.30)

project(AntivirusService)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Отдельная сборка
add_subdirectory(lib/logger)
add_subdirectory(lib/curl-8.11.0)
add_subdirectory(lib/account_manager)
add_subdirectory(lib/os_services)
add_subdirectory(lib/license_service)
add_subdirectory(lib/scan_engine)

add_subdirectory(lib/av_bases)

# Исходные файлы
set(SOURCE_FILES
    "apps/antimalware_service_app/src/SvcInit.cpp"
    "apps/antimalware_service_app/src/SvcThread.cpp"
    # "apps/antimalware_service_app/src/PipeService.cpp"
)

# Исполняемый файл
add_executable(AntivirusService ${SOURCE_FILES})

# Директории с .h файлами
target_include_directories(AntivirusService PRIVATE
    # "${CMAKE_SOURCE_DIR}/lib/curl-8.11.0/builds/libcurl-vc-x86-release-static-ipv6-sspi-schannel/include"
    "${CMAKE_SOURCE_DIR}/lib/logger/include"
    "${CMAKE_SOURCE_DIR}/lib/curl-8.11.0/include"
    "${CMAKE_SOURCE_DIR}/lib/account_manager/include"
    "${CMAKE_SOURCE_DIR}/lib/os_services/include"
    "${CMAKE_SOURCE_DIR}/lib/license_service/include"
    "${CMAKE_SOURCE_DIR}/lib/scan_engine/include"

    "${CMAKE_SOURCE_DIR}/lib/av_bases/include"
)

# Линковка библиотек
target_link_libraries(AntivirusService 
    Logger                # Библиотека Logger
    libcurl               # Библиотека cURL
    account_manager       # Библиотека Account Manager
    av_bases              # Библиотека AV Bases
    license_service       # Библиотека License Service
    os_services           # Библиотека OS Services
    scan_engine           # Библиотека Scan Engine
    Ws2_32                # Библиотека Windows Sockets
    Crypt32               # Библиотека шифрования
    Wldap32               # LDAP API
    Normaliz              # Unicode Normalization API
)


# cmake ..
# cmake --build . --config Release


# Сборка стартера (с SecureDesktop)
set(STARTER_SOURCE_FILES
    "apps/starter/src/Starter.cpp"
)
add_executable(Starter ${STARTER_SOURCE_FILES})






