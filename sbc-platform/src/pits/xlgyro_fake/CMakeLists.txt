project(xlgyro_fake_input LANGUAGES C)
find_package(Curses REQUIRED)
add_executable(xlgyro_fake_input main.c)
target_include_directories(xlgyro_fake_input PRIVATE ${CURSES_INCLUDE_DIR})
target_link_libraries(xlgyro_fake_input ${CURSES_LIBRARY})

