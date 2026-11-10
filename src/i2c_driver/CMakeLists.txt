add_library(i2c_driver STATIC
            i2c_driver.c)

target_include_directories(i2c_driver PUBLIC include)