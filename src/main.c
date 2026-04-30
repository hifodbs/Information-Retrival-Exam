#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/IO/one_timer_IO.h"
#include "console.h"
#include "test.h"
#include "IO/reduced_one_timer_IO.h"
#include "gui.h"

char *SOURCE_FILE = "cran/cran.all copy.1400";
char *BIN_FILE = "bin/reverse_index.bin";
char *BIN_REDUCED_FILE = "bin/r_reverse_index.bin";


/*
 * Starting point of the program,
 * By default it'll execute the comparison test
 * It's possible to pass one argument std or red to manual test the information retrival system
 */
int main(int argc, char *argv[]) {

    if (argc == 1) {

        srand(time(NULL));
        test_compare(SOURCE_FILE, BIN_FILE, BIN_REDUCED_FILE);

    } else if (strcmp(argv[1], "--gui") == 0) {

        return gui_run(argc, argv);

    } else if (strcmp(argv[1], "std") == 0) {

        if (create_posting_list(SOURCE_FILE, BIN_FILE) != 0) {
            printf("Error creating posting list\n");
            return -1;
        }
        start_IR(BIN_FILE,0);

    } else if (strcmp(argv[1], "red") == 0) {

        if (create_reduced_posting_list(SOURCE_FILE, BIN_REDUCED_FILE) != 0) {
            printf("Error creating posting list\n");
            return -1;
        }
        start_IR(BIN_REDUCED_FILE,1);
    }
    else {
        printf("Bad arguments\n");
    }

    return 0;
}
