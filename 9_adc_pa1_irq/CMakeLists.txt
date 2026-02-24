#1.Declare minimum CMake version
cmake_minimum_required(VERSION 3.20.0)
#2.Locate the zephyr build system as a CMake package
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
#3.Name the CMakeLists.txt project
project(adc_pa1_irq)
#4.Add the executable for the application, Tell zephyr which source files belong to this application
target_sources(app PRIVATE src/main.c)