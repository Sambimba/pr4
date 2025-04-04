cmake_minimum_required(VERSION 3.10)
project(guess_number_game)

set(CMAKE_C_STANDARD 11)

# Для клиента
add_executable(client client.c)
target_link_libraries(client)

# Для сервера
add_executable(server server.c)
target_link_libraries(server)
