project(PhoenixModules)

set(modulesPath ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE moduleFiles "*.*")

add_custom_target(${PROJECT_NAME}
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${modulesPath} ${CMAKE_BINARY_DIR}/mods
				   SOURCES ${moduleFiles}
)
