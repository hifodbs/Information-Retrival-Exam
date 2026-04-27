//
// Created by juan on 21/04/2026.
//

#include "../include/utils/encoder_decoder.h"

#include <stdlib.h>
#include <string.h>

/**
 *
 * @param i Integer to encode
 * @param len Outer parameter: number of byte needed for the encode
 * @return encoded integer
 * @details it used a reverse byte encode because i found it more practical to use
 */
unsigned char *encode_int(unsigned int i, int *len) {
    if (i < 128) *len = 1;
    else if (i < 16384) *len = 2;
    else if (i < 2097152) *len = 3;
    else if (i < 268435456) *len = 4;
    else *len = 5;

    char *result = calloc(*len, sizeof(char));

    int pos = 0;
    while (i >= 128) {
        result[pos++] = (i & 0x7F) | 0x80;
        i >>= 7;
    }

    result[pos] = (i & 0x7F);

    return result;
}

/**
 *
 * @param c encoded integer
 * @param len length of the ecnoded integer
 * @return Integer decoded
 */
unsigned int decode_int(unsigned char *c, int *len) {
    int result = 0;
    int i = 0;
    do {
        result |= (*c & 0x7F) << (7 * i++);
    } while (*c++ & 0x80);
    *len = i;
    return result;
}


/**
 *
 * @param words Array of words to be encoded in an string
 * @param number_of_words Number of words to be encoded
 * @return Byte necessary for the encoding
 */
int get_size(char **words, int number_of_words) {
    int total = strlen(words[0]) + 1;

    for (int i = 1; i < number_of_words; i++) {
        int common = 0;
        int len_prev = strlen(words[i - 1]);
        int len_current = strlen(words[i]);
        int limit = len_prev < len_current ? len_prev : len_current;

        while (common < limit && words[i - 1][common] == words[i][common])
            common++;

        total += len_current - common + 2;
    }

    return total;
}

//one byte for the integers if one word exceed 127 char it corrupt the memory
/**
 *
 * @param words Array of words to be encoded in an string
 * @param number_of_words Number of words to be encoded
 * @return Encoded string
 */
char *encode_words(char **words, int number_of_words) {
    char *result = malloc(sizeof(char) * get_size(words, number_of_words));

    result[0] = (char) strlen(words[0]);
    memcpy(result + 1, words[0], strlen(words[0]));
    int pos = strlen(words[0]) + 1;

    for (int i = 1; i < number_of_words; i++) {
        int common_char = 0;
        int max_i = strlen(words[i - 1]) < strlen(words[i]) ? strlen(words[i - 1]) : strlen(words[i]);
        for (int j = 0; j < max_i; j++)
            if (words[i - 1][j] == words[i][j])
                common_char++;
            else
                break;
        result[pos++] = (char) strlen(words[i]);
        result[pos++] = (char) common_char;
        memcpy(result + pos, words[i] + common_char, sizeof(char) * (strlen(words[i]) - common_char));
        pos += strlen(words[i]) - common_char;
    }
    return result;
}

/**
 *
 * @param coded Encoded string of the words
 * @param len How many words are encoded in the string
 * @return Array of words decoded
 */
char **decode_words(char *coded, int len) {
    char **result = malloc(sizeof(char *)*len);
    result[0] = malloc(sizeof(char)*((int)coded[0]+1));
    result[0][(int)coded[0]] = '\0';
    memcpy(result[0],coded+1,sizeof(char)*(int)coded[0]);
    int pos = (int)coded[0]+1;
    for (int i = 1 ; i < len;i++) {
        int word_len = (int)coded[pos++];
        char *c = malloc(sizeof(char)*(word_len+1));
        c[word_len] = '\0';
        int common = (int)coded[pos++];
        memcpy(c,result[i-1],sizeof(char)*common);
        memcpy(c+common,coded+pos,word_len-common);
        pos +=word_len-common;
        result[i] = c;
    }
    return result;
}
