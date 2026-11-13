cmake_minimum_required(VERSION 3.31)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(Subscale C CXX)

add_subdirectory(src/i2c_driver)
add_subdirectory(src/bmp581_lib)
add_subdirectory(src/gpsPA1010D_lib)
add_subdirectory(src/icm20948_lib)
add_subdirectory(src/drv8848)
add_subdirectory(src/LSM6DSOX)
add_subdirectory(test)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

add_executable(subscale main.cpp)

target_link_libraries(subscale PRIVATE i2c_driver)
target_link_libraries(subscale PRIVATE bmp581_lib)
target_link_libraries(subscale PRIVATE gpsPA1010D_lib)
target_link_libraries(subscale PRIVATE icm20948_lib)
target_link_libraries(subscale PRIVATE drv8848)
target_link_libraries(subscale PRIVATE LSM6DSOX)

target_include_directories(subscale PRIVATE ${LIBCAMERA_INCLUDE_DIRS})
target_link_libraries(subscale PRIVATE ${LIBCAMERA_LIBRARIES})
