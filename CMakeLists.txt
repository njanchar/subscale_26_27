cmake_minimum_required(VERSION 3.10)
project(Subscale C)

add_subdirectory(src/i2c_driver)
add_subdirectory(src/bmp581_lib)
add_subdirectory(src/gpsPA1010D_lib)
add_subdirectory(src/icm20948_lib)

add_executable(subscale main.c)

target_link_libraries(subscale PRIVATE i2c_driver)
target_link_libraries(subscale PRIVATE bmp581_lib)
target_link_libraries(subscale PRIVATE gpsPA1010D_lib)
target_link_libraries(subscale PRIVATE icm20948_lib)