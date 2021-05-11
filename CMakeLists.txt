cmake_minimum_required(VERSION 3.12)

project(
    Phoenix
    VERSION 0.2
    LANGUAGES CXX)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# Fail if running on MacOSX and the version is too low.
# TODO may end up using boost to get around this. So long as it doesn't
#      cause too much bloat.
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_SYSTEM_VERSION VERSION_LESS "10.15")
    message(FATAL_ERROR "MAC OS version ${CMAKE_SYSTEM_VERSION}, std::filesystem is only available on MACOS 10.15 or newer.")
endif()

# Use folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

target_include_directories(${PROJECT_NAME} PRIVATE src)
add_subdirectory(src)
# NOTE: this looks important find out what it does.
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		$<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
)
