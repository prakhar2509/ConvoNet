# Specify the minimum version of CMake required to build this project.
cmake_minimum_required(VERSION 3.10)

# Set the project name.
project(ChatApplication CXX)

# Set the C++ standard to C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the threads library, which is necessary for multithreading.
find_package(Threads REQUIRED)

# Find the OpenSSL library.
# This is required for SSL/TLS encryption.
find_package(OpenSSL REQUIRED)

# --- Server Executable ---
# Add an executable target named 'server'.
add_executable(server
    server/main.cpp
    server/Server.cpp
    server/Server.h
)

# Link the server executable against the threads and OpenSSL libraries.
target_link_libraries(server PRIVATE Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

# Add Windows-specific socket library for the server
if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()

# --- NEW: Copy certificate files next to the server executable after building ---
add_custom_command(TARGET server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/server/cert.pem"
        "$<TARGET_FILE_DIR:server>"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/server/key.pem"
        "$<TARGET_FILE_DIR:server>"
    COMMENT "Copying certificate files to output directory"
)


# --- Client Executable ---
# Add another executable target named 'client'.
add_executable(client
    client/main_client.cpp
    client/Client.cpp
    client/Client.h
)

# Link the client executable against the threads and OpenSSL libraries.
target_link_libraries(client PRIVATE Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

# Add Windows-specific socket library for the client
if(WIN32)
    target_link_libraries(client PRIVATE ws2_32)
endif()

# Optional: Print a message after configuration is done.
message(STATUS "CMake configuration complete. You can now build with 'make' or your chosen build tool.")
