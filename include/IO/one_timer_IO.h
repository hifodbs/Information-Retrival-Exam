#ifndef ONE_TIMER_IO_H
#define ONE_TIMER_IO_H
#include "utils/reverse_word.h"

linked_list get_words_and_idx(char *path);

void sort_words(int N, linked_list wordList, char **words, int **idx, int *size);

int create_posting_list(char *source_file, char* bin_file);


#endif