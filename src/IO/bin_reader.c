#include <stdio.h>
#include <stdlib.h>

#include "console.h"
#include "queryparser.h"
#include "utils/encoder_decoder.h"

FILE *rptr = NULL;

/**
 *
 * @param filename File to open
 * @details if a file is already open does nothing
 */
void init_bin_reader(char *filename) {
    if (rptr != NULL)
        return;
    rptr = fopen(filename, "rb");
    if (rptr == NULL) {
        perror("Error opening binary file");
    }
}

/**
 * @details cloase a file if there is a file opened
 */
void close_bin_reader() {
    if (rptr != NULL)
        fclose(rptr);
    rptr = NULL;
}

/**
 *
 * @param pos Position in the bin file where to start read
 * @param len Output parameter: Number of ids red from the file
 * @return Array of ids red
 */
int *get_idxs_from_bin(long pos, int *len) {
    if (rptr == NULL) return NULL;
    int lentgh = 0;
    int *ids;
    fseek(rptr, pos, SEEK_SET);
    fread(&lentgh, sizeof(int), 1, rptr);

    ids = malloc(sizeof(int) * lentgh);
    fread(ids, sizeof(int), lentgh, rptr);

    *len = lentgh;
    return ids;
}

/**
 *
 * @param id Id to check if is already in the list
 * @param isd Already allocated array where to check/put ids
 * @param id_len Length of the array of all ids
 */
void add_id(int id, int *isd, int *id_len) {
    int prev = id;
    int temp;
    for (int i = 0; i < *id_len; i++) {
        if (isd[i] == prev)
            return;
        if (isd[i] > prev) {
            temp = prev;
            prev = isd[i];
            isd[i] = temp;
        }
    }
    isd[*(id_len)] = prev;
    *(id_len) += 1;
}

/**
 *
 * @param d outer parameter: return a populate dictionary
 * @param is outer parameter: return an array of all Ids
 * @param len outer parameter: return the number of all Ids
 */
void get_dict_and_ids(dictionary *d, int **is, int *len) {
    if (rptr == NULL) return;
    dictionary dict = malloc(sizeof(struct _dictionary));
    if (dict == NULL)
        return;

    int N;
    fseek(rptr, 0, SEEK_SET);
    fread(&N, sizeof(int), 1, rptr);


    dict->word = malloc(sizeof(char *) * N);
    dict->pos = malloc(sizeof(size_t) * N);
    int *isd = malloc(sizeof(int) * N); //N for an upper bound (idea: #words>#ids)
    if (isd == NULL)
        return;

    int word_len;
    int id;
    int id_len = 0;
    dict->size = N;
    for (int i = 0; i < N; i++) {
        fread(&word_len, sizeof(int), 1, rptr);
        dict->word[i] = malloc(sizeof(char) * word_len + 1);
        fread(dict->word[i], sizeof(char), word_len, rptr);

        dict->word[i][word_len] = '\0';
        dict->pos[i] = ftell(rptr);
        fread(&word_len, sizeof(int), 1, rptr);
        for (int j = 0; j < word_len; j++) {
            fread(&id, sizeof(int), 1, rptr);
            add_id(id, isd, &id_len);
        }
    }

    *d = dict;
    *len = id_len;
    isd = realloc(isd, sizeof(int) * id_len);
    *is = isd;
}

//---------------------REDUCED BIN READER------------------------

/**
 *
 * @param d outer parameter: return a populate dictionary
 * @param is outer parameter: return an array of all Ids
 * @param len outer parameter: return the number of all Ids
 * @details made it for the reduced algorithm
 */
void reduced_get_dict_and_ids(dictionary *d, int **is, int *len) {
    if (rptr == NULL) return;
    dictionary dict = malloc(sizeof(struct _dictionary));
    if (dict == NULL)
        return;

    int N;
    fseek(rptr, 0, SEEK_SET);
    fread(&N, sizeof(int), 1, rptr);


    dict->word = malloc(sizeof(char *) * ((N + BLOCK_SIZE - 1) / BLOCK_SIZE));
    dict->pos = malloc(sizeof(size_t) * N);
    int *isd = malloc(sizeof(int) * N);
    if (isd == NULL)
        return;

    int word_len;
    int id_len = 0;
    dict->size = N;
    char **words = malloc(sizeof(char *) * BLOCK_SIZE);
    for (int i = 0; i < N; i++) {
        fread(&word_len, sizeof(int), 1, rptr);
        char *word = malloc(sizeof(char) * word_len + 1);
        fread(word, sizeof(char), word_len, rptr);

        word[word_len] = '\0';
        words[i % BLOCK_SIZE] = word;

        if ((i + 1) % BLOCK_SIZE == 0) {
            dict->word[i / BLOCK_SIZE] = encode_words(words,BLOCK_SIZE);
            for (int a = 0; a < BLOCK_SIZE; a++)
                free(words[a]);
        }

        dict->pos[i] = ftell(rptr);
        fread(&word_len, sizeof(int), 1, rptr);
        char *ids = malloc(sizeof(char) * word_len);
        fread(ids, sizeof(char), word_len, rptr);
        int pos_ids = 0;
        int step = 0;
        int last = 0;
        int id = 0;
        while (pos_ids < word_len) {
            id = decode_int(ids + pos_ids, &step)+last;
            add_id(id, isd, &id_len);
            pos_ids += step;
            last = id;
        }
        free(ids);
    }
    if (N % BLOCK_SIZE != 0) {
        dict->word[N / BLOCK_SIZE] = encode_words(words, N % BLOCK_SIZE);
        for (int a = 0; a < N % BLOCK_SIZE; a++)
            free(words[a]);
    }
    free(words);

    *d = dict;
    *len = id_len;
    isd = realloc(isd, sizeof(int) * id_len);
    *is = isd;
}

/**
 *
 * @param pos Position in the bin file where to start read
 * @param len Output parameter: Number of ids red from the file
 * @return Array of ids red
 * @details it have to decode the information in the bin file
 */
int *reduced_get_idxs_from_bin(long pos, int *len) {
    if (rptr == NULL) return NULL;
    int char_length = 0;
    fseek(rptr, pos, SEEK_SET);
    fread(&char_length, sizeof(int), 1, rptr);

    char *char_ids = malloc(sizeof(char) * char_length);
    fread(char_ids, sizeof(char), char_length, rptr);

    int length = 0;
    for (int i = 0; i < char_length; i++)
        if (!(char_ids[i] & 0x80))
            length++;

    int *ids = malloc(sizeof(int) * length);
    int pos_ids = 0;
    int step = 0;
    int last = 0;
    for (int i = 0; pos_ids < char_length; i++) {
        ids[i] = decode_int(char_ids + pos_ids, &step)+last;
        pos_ids += step;
        last = ids[i];
    }

    free(char_ids);

    *len = length;
    return ids;
}
