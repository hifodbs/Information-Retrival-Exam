//
// Created by juan on 23/04/2026.
//

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "queryparser.h"
#include "IO/bin_reader.h"
#include "IO/one_timer_IO.h"
#include "IO/reduced_one_timer_IO.h"
#include "utils/encoder_decoder.h"

int N_QUERIES = 100;

/**
 *
 * @param words All possible words
 * @param N Number of words
 * @param end Probability of return a End point (Term) much higher end much probability to bhe chose. it's guarantee at end = 10
 * @return Return a query
 */
char *create_query(char **words, int N, int end) {
    int choice = end < 4 ? rand() % 4 : rand() % (4 + end);
    char *c1, *c2, *result;

    if (choice > 3 || end >= 10) {
        c1 = words[rand() % N];
        int len_c1 = strlen(c1);
        result = malloc(len_c1 + 1 + 1);

        memcpy(result, c1, len_c1);
        memcpy(result + len_c1, " \0", 2);
        return result;
    }

    if (choice == 0) {
        c1 = create_query(words, N, end + 10);
        if (strcmp(c1, "not") == 0)
            return c1;
        result = malloc(sizeof(char) * (strlen(c1) + 4 + 1));
        result[strlen(c1) + 4] = '\0';
        memcpy(result, "not ", 4 * sizeof(char));
        memcpy(result + 4, c1, strlen(c1) * sizeof(char));
        free(c1);
        return result;
    }
    if (choice == 1) {
        c1 = create_query(words, N, end + 2);
        int lec_c1 = strlen(c1);
        result = malloc(sizeof(char) * (lec_c1 + 4 + 1));
        result[lec_c1 + 4] = '\0';
        memcpy(result, "( ", 2 * sizeof(char));
        memcpy(result + 2, c1, lec_c1 * sizeof(char));
        memcpy(result + 2 + lec_c1, ") ", 2 * sizeof(char));
        free(c1);
        return result;
    }

    c1 = create_query(words, N, end + 1);
    c2 = create_query(words, N, end + 1);
    int len_c1 = strlen(c1);
    int len_c2 = strlen(c2);

    if (choice == 2) {
        result = malloc(sizeof(char) * (len_c1 + 3 + len_c2 + 1));
        result[len_c1 + 3 + len_c2] = '\0';
        memcpy(result, c1, len_c1 * sizeof(char));
        memcpy(result + len_c1, "or ", 3 * sizeof(char));
        memcpy(result + len_c1 + 3, c2, len_c2 * sizeof(char));
    } else {
        result = malloc(sizeof(char) * (len_c1 + 4 + len_c2 + 1));
        result[len_c1 + 4 + len_c2] = '\0';
        memcpy(result, c1, len_c1 * sizeof(char));
        memcpy(result + len_c1, "and ", 4 * sizeof(char));
        memcpy(result + len_c1 + 4, c2, len_c2 * sizeof(char));
    }
    free(c1);
    free(c2);
    return result;
}

/**
 *
 * @param source_file the file containing the documents and from what read the words
 * @return A list of queries
 */
char **create_queries(char *source_file) {
    linked_list word_list = get_words_and_idx(source_file);
    if (word_list == NULL)
        return NULL;

    int N = lst_size(word_list);
    char *words[N];
    int *idx[N];
    int size[N];

    sort_words(N, word_list, words, idx, size);

    lst_delete(word_list);
    char **queries = malloc(sizeof(char *) * N_QUERIES);
    for (int i = 0; i < N_QUERIES; i++) {
        queries[i] = create_query(words, N, 1);
    }
    for (int i = 0; i < N; i++) {
        free(words[i]);
        free(idx[i]);
    }
    return queries;
}

// Size = last position of the file
long get_file_size(char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
        return -1;
    fseek(fp, 0,SEEK_END);
    long result = ftell(fp);
    fclose(fp);
    return result;
}


//Self explain
void print_section(char *s) {
    printf("\n");
    int i = (80 - strlen(s)) / 2;
    for (int a = 0; a < i; a++)
        printf("-");
    printf("%s", s);
    for (int a = 0; a < i + (strlen(s) % 2); a++)
        printf("-");
    printf("\n");
}


//Before comparing the size it create the files
bool compare_bin_size(char *source, char *bin_file, char *reduce_bin_file) {
    print_section("CREATING FILES");
    printf("Creating bin file...\n");
    if (create_posting_list(source, bin_file) != 0) {
        printf("Error creating posting list\n");
        return true;
    }

    printf("Creating reduced bin file...\n");
    if (create_reduced_posting_list(source, reduce_bin_file) != 0) {
        printf("Error creating posting list\n");
        return true;
    }

    long size = get_file_size(bin_file);
    long reduce_size = get_file_size(reduce_bin_file);
    print_section("FILE SIZE COMPARISON");
    printf("Non reduced\tReduced\n");
    printf("%ldB        \t%ldB\n", size, reduce_size);
    printf("Byte saved: %ld for a percetage save of %.2f%%\n", size - reduce_size,
           (1 - (double) reduce_size / (double) size) * 100);
    return false;
}

//Self explain
long get_size_red_dict(dictionary red_d) {
    long red_dict_size = sizeof(red_d->size);
    red_dict_size += sizeof(red_d->word);
    red_dict_size += sizeof(red_d->pos);
    red_dict_size += sizeof(long) * red_d->size;
    red_dict_size += sizeof(char *) * ((red_d->size + BLOCK_SIZE - 1) / BLOCK_SIZE);
    for (int i = 0; i < (red_d->size + BLOCK_SIZE - 1) / BLOCK_SIZE; i++) {
        int n_words = (red_d->size - i * BLOCK_SIZE < BLOCK_SIZE) ? (red_d->size - i * BLOCK_SIZE) : BLOCK_SIZE;
        char **c = decode_words(red_d->word[i], n_words);
        red_dict_size += get_size(c, n_words);
        for (int a = 0; a < n_words; a++)
            free(c[a]);
        free(c);
    }
    return red_dict_size;
}

//Self explain
long get_size_dict(dictionary std_d) {
    long std_dict_size = sizeof(std_d->size);
    std_dict_size += sizeof(std_d->word);
    std_dict_size += sizeof(std_d->pos);
    std_dict_size += sizeof(long) * std_d->size;
    std_dict_size += sizeof(char *) * std_d->size;
    for (int i = 0; i < std_d->size; i++)
        std_dict_size += strlen(std_d->word[i]) + 1;
    return std_dict_size;
}

// Check if the results are the same for each querie from the two alogirthm
void check_correctness(int **std_idx, int *std_len, int **red_idx, int *red_len) {
    print_section("CHECK CORRECTNESS OF RESULTS");
    int number_of_errors = 0;
    for (int i = 0; i < N_QUERIES; i++) {
        if (std_len[i] != red_len[i])
            for (int j = 0; j < std_len[i]; j++)
                if (std_idx[i][j] != red_idx[i][j])
                    number_of_errors++;
    }
    printf("Erros found %d\n", number_of_errors);
}

//Self explain
void compare_dict_size(dictionary std_d, dictionary red_d) {
    print_section("DICTIONARY COMPARISON");
    long std_dict_size = get_size_dict(std_d);
    long red_dict_size = get_size_red_dict(red_d);

    printf("Non reduced\tReduced\n");
    printf("%ldB        \t%ldB\n", std_dict_size, red_dict_size);
    printf("Byte saved: %ld for a percetage save of %.2f%%\n", std_dict_size - red_dict_size,
           (1 - (double) red_dict_size / (double) std_dict_size) * 100);
}

//Self explain
void compare_avg_time(float std_ms, float red_ms) {
    float avg_std_ms = std_ms / (float) N_QUERIES;
    float avg_red_ms = red_ms / (float) N_QUERIES;
    print_section("AVG TIME PER QUERY");
    printf("Non reduced\tReduced\n");
    printf("%fms        \t%fms\n", avg_std_ms, avg_red_ms);
    printf("Average time saved: %fms for a percetage save of %.2f%%\n", avg_std_ms - avg_red_ms,
           (1 - (double) avg_red_ms / (double) avg_std_ms) * 100);
}

// simulate processing of N_QUERIES and save stats
// The algorithm si choose with arg mode
// mode = 0 standard algorithm
// mode = 1 reduced algorithm
// output parameters: red_d, red_idx, red_len
// it returns the total time for processing the queries
float run_algorithm(char *file, char **queries, dictionary *red_d, int **red_idx, int *red_len, int mode) {
    init_bin_reader(file);
    init_query_parser(mode);
    clock_t start = clock();
    for (int a = 0; a < N_QUERIES; a++) {
        char *q_copy = strdup(queries[a]);
        query_parser(&red_idx[a], &red_len[a], q_copy);
        free(q_copy);
    }
    clock_t end = clock();
    close_bin_reader();
    float red_ms = (end - start) / (CLOCKS_PER_SEC / 1000.0);
    *red_d = get_dict();
    return red_ms;
}

// just for avoiding memory leaks
void free_resources(char **queries, int **std_idx, int *std_len, int **red_idx, int *red_len) {
    for (int a = 0; a < N_QUERIES; a++) {
        free(std_idx[a]);
        free(red_idx[a]);
        free(queries[a]);
    }
    free(std_idx);
    free(std_len);
    free(red_idx);
    free(red_len);
    free(queries);
}

// Only visible function of this file
// Execute various test for comparing the two algorithms (standard and reduces)
void test_compare(char *source, char *bin_file, char *reduce_bin_file) {
    print_section("CREATING QUERIES");
    char **queries = create_queries(source);
    printf("Create %d queries\n", N_QUERIES);

    if (compare_bin_size(source, bin_file, reduce_bin_file)) return;

    print_section("RUNNING ALGORITHMS");
    printf("Running standard algorithm...\n");
    dictionary std_d = NULL;
    int **std_idx = malloc(sizeof(int *) * N_QUERIES);
    int *std_len = malloc(sizeof(int) * N_QUERIES);
    float std_ms = run_algorithm(bin_file, queries, &std_d, std_idx, std_len, 0);


    printf("Running enhanced algorithm...\n");
    dictionary red_d = NULL;
    int **red_idx = malloc(sizeof(int *) * N_QUERIES);
    int *red_len = malloc(sizeof(int) * N_QUERIES);
    float red_ms = run_algorithm(reduce_bin_file, queries, &red_d, red_idx, red_len, 1);

    check_correctness(std_idx, std_len, red_idx, red_len);

    compare_dict_size(std_d, red_d);

    compare_avg_time(std_ms, red_ms);

    free_resources(queries, std_idx, std_len, red_idx, red_len);
}
