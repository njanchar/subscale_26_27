add_library(icm20948_lib STATIC
            icm20948_lib.c)

target_include_directories(icm20948_lib PUBLIC include)
target_link_libraries(subscale PRIVATE i2c_driver)