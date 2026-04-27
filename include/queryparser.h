#ifndef IR_QUERYPARSER_H
#define IR_QUERYPARSER_H

enum Token_type {OR,AND,NOT,LEFT_PAR,RIGHT_PAR,TERM};

struct _token
{
    int * indexes;
    int len;
    enum Token_type type;
};

struct _dictionary
{
    char **word;
    long *pos;
    int size;
};

typedef struct _dictionary *dictionary;

typedef struct _token *token;

void query_parser(int**indexes, int *len, char *q);

void init_query_parser( int mode);

dictionary get_dict();

#endif //IR_QUERYPARSER_H
