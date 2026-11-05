cmake_minimum_required(VERSION 3.10)
project(Subscale C)

add_subdirectory(i2c_driver)

add_executable(subscale main.c)

target_link_libraries(subscale PRIVATE i2c_driver)