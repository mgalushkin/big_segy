cmake_minimum_required( VERSION  3.10 )

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set( CMAKE_INCLUDE_CURRENT_DIR  ON )
set( CMAKE_CXX_STANDARD  17 )


project( generate_big_segy )


find_package( segyio  REQUIRED )


file( GLOB_RECURSE  HEADERS  "*.hpp"  "*.h" )
file( GLOB_RECURSE  SOURCES  "*.cpp"  "*.c" )


add_executable( ${PROJECT_NAME} ${HEADERS}  ${SOURCES} )
target_link_libraries( ${PROJECT_NAME} segyio::segyio )
