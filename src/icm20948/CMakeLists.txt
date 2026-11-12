add_library(icm20948 STATIC
            icm20948.c)

target_include_directories(icm20948 PUBLIC include)
target_link_libraries(icm20948 PRIVATE i2c_driver)