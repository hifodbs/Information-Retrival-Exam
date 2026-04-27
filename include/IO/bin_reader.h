#ifndef BIN_READER_H
#define BIN_READER_H
#include "queryparser.h"

void init_bin_reader(char *filename);

void close_bin_reader();

int* get_idxs_from_bin(long pos, int*len);

void get_dict_and_ids(dictionary *d, int **is, int *len);


void reduced_get_dict_and_ids(dictionary *d, int **is, int *len);

int *reduced_get_idxs_from_bin(long pos,int *len);

#endif // BIN_READER_H