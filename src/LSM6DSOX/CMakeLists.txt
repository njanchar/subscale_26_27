add_library(lsm6dsv16x STATIC
            lsm6dsv16x.c)

target_include_directories(lsm6dsv16x PUBLIC include)
target_link_libraries(lsm6dsv16x PRIVATE i2c_driver)
