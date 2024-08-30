cmake_minimum_required(VERSION 3.10)

project(FileUploadProject)

set(CMAKE_C_STANDARD 11)

# 查找 OpenSSL 和 hiredis 库
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

include_directories(${OPENSSL_INCLUDE_DIR} ${HIREDIS_INCLUDE_DIRS})

# 添加子目录
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(server)