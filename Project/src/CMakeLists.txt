cmake_minimum_required(VERSION 3.0.0)

# add_subdirectory(Terminal Utils)

file(GLOB_RECURSE Root_SOURCES CONFIGURE_DEPENDS "*.c")

include_directories(${MyVim_SOURCE_DIR}/Project/include)
add_executable(TextEditor ${Root_SOURCES})
add_custom_command(TARGET TextEditor POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy TextEditor ../../../TextEditor)
