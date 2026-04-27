//
// Created by juan on 21/04/2026.
//
#include <stdlib.h>

#include "../../include/IO/reduced_one_timer_IO.h"

#include <stdio.h>
#include <string.h>

#include "IO/one_timer_IO.h"
#include "utils/encoder_decoder.h"
#include "utils/reverse_word.h"

/**
 *
 * @param idxs Ids to encode in a string
 * @param size How many ids there are
 * @return Number of byte represnting all the ids
 */
int get_byte_necessary(int *idxs, int size) {
    int n = 0;
    int last = 0;
    for (int i = 0; i < size; i++) {
        if (idxs[i]-last < 128) n += 1;
        else if (idxs[i]-last < 16384) n += 2;
        else if (idxs[i]-last < 2097152) n += 3;
        else if (idxs[i]-last < 268435456) n += 4;
        else n += 5;
        last = idxs[i];
    }
    return n;
}

/**
 *
 * @param N Number of words
 * @param idx Ids for each word
 * @param size How many ids there are for each word
 * @param encoded_size Size occupied for each encoded list of ids for each word
 * @return Array of string representing the ids for each word
 * @details Are saved the difference of the ids from the previous one,
 * so the Ids it's to be calculate after the decoding phase
 */
char **encode_idx(int N, int **idx, int *size, int *encoded_size) {
    char **result = malloc(sizeof(char *) * N);

    for (int i = 0; i < N; i++) {
        char *id_list = malloc(sizeof(char) * get_byte_necessary(idx[i], size[i]));
        int encoded_len = 0;
        int last = 0;
        for (int j = 0; j < size[i]; j++) {
            int len;
            char *num = encode_int(idx[i][j]-last, &len);
            memcpy(id_list + encoded_len, num, len);
            encoded_len += len;
            free(num);
            last = idx[i][j];
        }
        encoded_size[i] = encoded_len;
        result[i] = id_list;
    }
    return result;
}

/**
 *
 * @param filename Path where to write the bin file
 * @param N How many words to write
 * @param words Array of words to write
 * @param idx Array of ids for each word
 * @param size How many ids for each word
 * @return 0 = OK, 1 = Error
 * @details create a binary file for each word it writes length of word,the word,how many ids have the word
 * end the encoded string of the Ids.
 * at the start of the file it writes how many words there will be
 */
int create_reduced_file(char *filename, int N, char **words, int **idx, int *size) {
    remove(filename);

    FILE *fptr;
    // Create a file
    fptr = fopen(filename, "wb");
    if (fptr == NULL)
        return 1;


    int *encode_size = calloc(sizeof(int), N);
    char **encoded_idx = encode_idx(N, idx, size, encode_size);

    fwrite(&N, sizeof(int), 1, fptr);
    for (int i = 0; i < N; i++) {
        int length = strlen(words[i]);
        fwrite(&length, sizeof(int), 1, fptr);
        fwrite(words[i], sizeof(char), length, fptr);
        fwrite(&encode_size[i], sizeof(int), 1, fptr);
        fwrite(encoded_idx[i], sizeof(char), encode_size[i], fptr);
        free(encoded_idx[i]);
    }
    free(encode_size);
    free(encoded_idx);
    fclose(fptr);
    return 0;
}

/**
 *
 * @param source_file Path to the source file
 * @param bin_file Path where to write the reduced bin file
 * @return 0 = OK, 1 = Error
 * @details full pipeline for creating a reduced posting list
 */
int create_reduced_posting_list(char *source_file, char *bin_file) {
    linked_list word_list = NULL;
    word_list = get_words_and_idx(source_file);
    if (word_list == NULL)
        return 1;

    int N = lst_size(word_list);
    char *words[N];
    int *idx[N];
    int size[N];

    sort_words(N, word_list, words, idx, size);

    lst_delete(word_list);

    int return_status = 0;

    if (create_reduced_file(bin_file, N, words, idx, size) != 0)
        return_status = 1;

    for (int i = 0; i < N; i++) {
        free(words[i]);
        free(idx[i]);
    }
    return return_status;
}
