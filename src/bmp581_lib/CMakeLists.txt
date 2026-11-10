add_library(bmp581_lib STATIC
            bmp581_lib.c)

target_include_directories(bmp581_lib PUBLIC include)
target_link_libraries(bmp581_lib PRIVATE i2c_driver)