cmake_minimum_required(VERSION 3.10)
project(IR C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(ADWAITA REQUIRED libadwaita-1)

include_directories(include ${GTK4_INCLUDE_DIRS} ${ADWAITA_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS} ${ADWAITA_LIBRARY_DIRS})

add_executable(IR
        include/IO/bin_reader.h
        include/console.h
        include/IO/one_timer_IO.h
        include/utils/reverse_word.h
        src/IO/bin_reader.c
        src/console.c
        src/main.c
        src/IO/one_timer_IO.c
        src/utils/reverse_word.c
        src/queryparser.c
        include/queryparser.h
        src/utils/stack.c
        include/utils/stack.h
        src/IO/reduced_one_timer_IO.c
        include/IO/reduced_one_timer_IO.h
        src/utils/encoder_decoder.c
        include/utils/encoder_decoder.h
        src/test.c
        include/test.h
        src/gui.c
        include/gui.h
        src/IO/doc_retriever.c
        include/IO/doc_retriever.h
)

target_link_libraries(IR m ${GTK4_LIBRARIES} ${ADWAITA_LIBRARIES})
