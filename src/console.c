#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "../include/IO/bin_reader.h"
#include "console.h"
#include "queryparser.h"


/**
 * @details if using the program in terminal it clears the screen
 */
void clear_screen() {
    if (isatty(STDOUT_FILENO)) {
        printf("\033c");
        fflush(stdout);
    }
}

// self explain
void print_menu() {
    printf("Starting Information Retrieval System...\n");
    printf("Write a query to search for (or 'exit' to quit) [not is not implemeted]:\n");
    printf("Example query: 'dog or (cat and fish)'\n");
}

// self explain
void print_result(int *result, int len) {
    printf("Results found: %d\n", len);
    for (int i = 0; i < len; i++)
    {
        printf("%d ", result[i]);
    }
    printf("\n");
}

/**
 *
 * @param filename bin file where the posting list is
 * @param mode standard algortihm 0, reduced algorithm 1
 * @details this functino is the interactive part of the system
 */
void start_IR(char *filename, int mode)
{
    init_bin_reader(filename);
    init_query_parser(mode);

    print_menu();

    char query[100];
    int *result;
    int len = 0;

    while (1)
    {
        printf("Query: ");
        if (fgets(query, sizeof(query), stdin) == NULL)
            break;

        for (size_t i = 0; i < strlen(query); i++)
            *(query + i) = tolower(*(query + i));

        if (strcmp(query, "exit\n") == 0)
            break;

        query_parser(&result,&len,query);

        clear_screen();
        print_menu();

        print_result(result, len);
        free(result);
    }

    close_bin_reader();
}
