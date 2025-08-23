# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\zephyr_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\zephyr_autogen.dir\\ParseCache.txt"
  "zephyr_autogen"
  )
endif()
