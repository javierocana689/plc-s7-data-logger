# CMakeLists.txt — Build del PLC S7 Data Logger (C++)
#
# Compilación típica:
#   cmake -S . -B build
#   cmake --build build
#
# Dependencias:
#   - nlohmann/json  -> se descarga automáticamente con FetchContent
#   - snap7 (nativo) -> lo pones tú en third_party/snap7/ (ver README)
cmake_minimum_required(VERSION 3.15)
project(plc_s7_data_logger_src3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencia 1: nlohmann/json (descarga automática) ---
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# --- Dependencia 2: snap7 (nativo, provisto por ti en third_party/snap7) ---
set(SNAP7_ROOT "${CMAKE_SOURCE_DIR}/../third_party/snap7" CACHE PATH "Ruta a snap7")

if(NOT EXISTS "${SNAP7_ROOT}/include/snap7.h")
    message(FATAL_ERROR
        "No se encontro snap7.h en ${SNAP7_ROOT}/include.\n"
        "Ver cpp/third_party/snap7/README.md "
        "(ver el README, seccion 'snap7').")
endif()

# --- Ejecutable ---
add_executable(plc_logger_src3
    src/main.cpp
    src/Config.cpp
    src/Logger.cpp
    src/FileManager.cpp
    src/PlcReader.cpp
)

target_include_directories(plc_logger_src3 PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SNAP7_ROOT}/include
)

target_link_directories(plc_logger_src3 PRIVATE ${SNAP7_ROOT}/lib)
target_link_libraries(plc_logger_src3 PRIVATE snap7 nlohmann_json::nlohmann_json)

# std::thread necesita pthread en Linux.
find_package(Threads REQUIRED)
target_link_libraries(plc_logger_src3 PRIVATE Threads::Threads)

# En Windows, copia la DLL de snap7 junto al ejecutable tras compilar.
if(WIN32 AND EXISTS "${SNAP7_ROOT}/lib/snap7.dll")
    add_custom_command(TARGET plc_logger_src3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SNAP7_ROOT}/lib/snap7.dll" "$<TARGET_FILE_DIR:plc_logger_src3>")
endif()
