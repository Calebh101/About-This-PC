# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "AboutThisPC_autogen"
  "CMakeFiles/AboutThisPC_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/AboutThisPC_autogen.dir/ParseCache.txt"
  )
endif()
