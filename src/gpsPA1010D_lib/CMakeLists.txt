add_library(gpsPA1010D_lib STATIC
            gpsPA1010D_lib.c)

target_include_directories(gpsPA1010D_lib PUBLIC include)
target_link_libraries(gpsPA1010D_lib PRIVATE i2c_driver)