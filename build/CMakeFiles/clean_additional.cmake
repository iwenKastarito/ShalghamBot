# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/perft_test_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/perft_test_autogen.dir/ParseCache.txt"
  "CMakeFiles/qtchess_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/qtchess_autogen.dir/ParseCache.txt"
  "perft_test_autogen"
  "qtchess_autogen"
  )
endif()
