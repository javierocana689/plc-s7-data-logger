# CMakeLists.txt — src2_security (contraseña de Full access, intento no funcional)
cmake_minimum_required(VERSION 3.15)
project(plc_s7_data_logger_src2_security LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

set(SNAP7_ROOT "${CMAKE_SOURCE_DIR}/../third_party/snap7" CACHE PATH "Ruta a snap7")

if(NOT EXISTS "${SNAP7_ROOT}/include/snap7.h")
    message(FATAL_ERROR
        "No se encontro snap7.h en ${SNAP7_ROOT}/include.\n"
        "Ver cpp/third_party/snap7/README.md")
endif()

add_executable(plc_logger_src2_security
    src/main.cpp
    src/Config.cpp
    src/ConfigValidator.cpp
    src/Logger.cpp
    src/FileManager.cpp
    src/PlcReader.cpp
)

target_include_directories(plc_logger_src2_security PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SNAP7_ROOT}/include
)

target_link_directories(plc_logger_src2_security PRIVATE ${SNAP7_ROOT}/lib)
target_link_libraries(plc_logger_src2_security PRIVATE snap7 nlohmann_json::nlohmann_json)

find_package(Threads REQUIRED)
target_link_libraries(plc_logger_src2_security PRIVATE Threads::Threads)

if(WIN32 AND EXISTS "${SNAP7_ROOT}/lib/snap7.dll")
    add_custom_command(TARGET plc_logger_src2_security POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SNAP7_ROOT}/lib/snap7.dll" "$<TARGET_FILE_DIR:plc_logger_src2_security>")
endif()
