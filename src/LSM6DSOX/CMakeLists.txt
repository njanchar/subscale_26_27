add_library(LSM6DSOX STATIC
            LSM6DSOX.c)

target_include_directories(LSM6DSOX PUBLIC include)
target_link_libraries(LSM6DSOX PRIVATE i2c_driver)