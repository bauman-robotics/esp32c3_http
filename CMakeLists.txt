# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32c3_http)

# Убедитесь, что путь к компонентам указан правильно
list(APPEND EXTRA_COMPONENT_DIRS components/protocol_examples_common)
list(APPEND EXTRA_COMPONENT_DIRS components/espressif__led_strip)
list(APPEND EXTRA_COMPONENT_DIRS components/esp-tls)
list(APPEND EXTRA_COMPONENT_DIRS components/esp_driver_i2c)
list(APPEND EXTRA_COMPONENT_DIRS components/i2c)
# set(EXTRA_COMPONENT_DIRS components)

