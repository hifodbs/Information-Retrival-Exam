cmake_minimum_required(VERSION 4.2)
project(IR C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

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
)

target_link_libraries(IR m)