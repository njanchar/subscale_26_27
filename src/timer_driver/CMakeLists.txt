add_library(timer_driver STATIC
            timer_driver.c)

target_include_directories(icm20948_lib PUBLIC include)