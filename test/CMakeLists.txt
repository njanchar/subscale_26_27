add_executable(icm_test icm_test.c)
add_executable(drv8848_test drv8848_test.c)

target_link_libraries(icm_test PRIVATE i2c_driver)
target_link_libraries(icm_test PRIVATE icm20948_lib)

target_link_libraries(drv8848_test PRIVATE i2c_driver)
target_link_libraries(drv8848_test PRIVATE drv8848)
target_link_libraries(drv8848_test PRIVATE timer_driver)
