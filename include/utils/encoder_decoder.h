//
// Created by juan on 21/04/2026.
//

#ifndef IR_ENCODER_DECODER_H
#define IR_ENCODER_DECODER_H

#define BLOCK_SIZE 4

unsigned char * encode_int(unsigned int i, int * len);

unsigned int decode_int(unsigned char *c, int *len);

int get_size(char **words, int number_of_words);

char *encode_words(char **words,int number_of_words);

char **decode_words(char *coded, int len);

#endif //IR_ENCODER_DECODER_H
