cmake_minimum_required(VERSION 3.15)

# Исправление опечатки и установка версии
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION ${PATCH_VERSION})

project(lb4 VERSION ${PROJECT_VERSION})

# Генерация version.h в директорию сборки (build), чтобы избежать загрязнения исходников
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)

# Настройки C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Добавляем директорию сборки в include-пути, чтобы найти сгенерированный version.h
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(header)

# Исходники
set(SRC_FILES main.cpp)

# Цель
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Установка
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# CPack — только для Windows-совместимых форматов
if(WIN32)
    set(CPACK_GENERATOR "ZIP")  # или "NSIS", если установлен Nullsoft Installer
else()
    set(CPACK_GENERATOR "DEB")
endif()

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "example@example.com")

include(CPack)