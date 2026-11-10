add_library(icm20948_lib STATIC
            icm20948_lib.c)

target_include_directories(icm20948_lib PUBLIC include)
target_link_libraries(icm20948_lib PRIVATE i2c_driver)