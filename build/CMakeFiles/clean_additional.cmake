# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/QDrawPro_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/QDrawPro_autogen.dir/ParseCache.txt"
  "QDrawPro_autogen"
  )
endif()
