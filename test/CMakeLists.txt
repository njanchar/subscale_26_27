add_executable(icm_test icm_test.c)

target_link_libraries(icm_test PRIVATE i2c_driver)
target_link_libraries(icm_test PRIVATE icm20948_lib)
