#include "queryparser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/IO/bin_reader.h"
#include "../include/utils/stack.h"
#include "utils/encoder_decoder.h"


dictionary dict = NULL;
int *all_ids = NULL;
int ids_len = 0;
int mode = 0;

// just for the funny debugging time
const char *Token_names[] = {
    "OR",
    "AND",
    "NOT",
    "LEFT_PAR",
    "RIGHT_PAR",
    "TERM"
};

/**
 *
 * @param word The word from what gather the Ids
 * @param len Outer parameter: how many Ids are found
 * @param ids Outer parameter: All the ids found
 */
void reduced_get_ids_from_word(char *word, int *len, int **ids) {
    char **words = NULL;
    int n_words = 0;
    for (int i = 0; i < dict->size; i++) {
        if (i % BLOCK_SIZE == 0) {
            free(words);
            n_words = (dict->size - i < BLOCK_SIZE) ? (dict->size - i) : BLOCK_SIZE;
            words = decode_words(dict->word[i / BLOCK_SIZE], n_words);
        }
        if (strcmp(words[i % BLOCK_SIZE], word) == 0) {
            int *new_ids;
            new_ids = reduced_get_idxs_from_bin(dict->pos[i], len);
            free(*ids);
            for (int j = i % BLOCK_SIZE; j < n_words; j++)
                free(words[j]);
            free(words);
            *ids = new_ids;
            return;
        }
        free(words[i % BLOCK_SIZE]);
    }
    free(words);
    *len = 0;
    *ids = NULL;
}

/**
 *
 * @param word The word from what gather the Ids
 * @param len Outer parameter: how many Ids are found
 * @param ids Outer parameter: All the ids found
 */
void get_ids_from_word(char *word, int *len, int **ids) {
    for (int i = 0; i < dict->size; i++) {
        if (strcmp(dict->word[i], word) == 0) {
            int *new_ids;
            new_ids = get_idxs_from_bin(dict->pos[i], len);
            free(*ids);
            *ids = new_ids;
            return;
        }
    }
    *len = 0;
    *ids = NULL;
}

/**
 *
 * @param query The query to tell how may words there are
 * @return Number of words inside a query
 */
int get_number_of_words(const char *query) {
    int counted = 0;
    const char *it = query;
    int inword = 0;

    do
        switch (*it) {
            case '\0':
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if (inword) {
                    inword = 0;
                    counted++;
                }
                break;
            default: inword = 1;
        } while (*it++);

    return counted;
}

/**
 *
 * @param q The starting query
 * @param token_list Outer parameter: The tokenize query
 * @details from each word tags the purpose (TERM, OR, AND, ecc...)
 * it contains some logic too, i.e. the second not is threatened as a TERM
 * (same for other operation withou '(' and ')' .)
 * if the token is a term it gathers the ids from that word
 * @attention destructive function, it uses strtok the q param is gonna be modified
 */
void tokenize(char *q, token *token_list) {
    char *word = strtok(q, " \n");
    int prev_op = 1;
    int prev_not = 0;
    int i = 0;
    while (word != NULL) {
        token t = malloc(sizeof(struct _token));
        t->len = 0;
        t->indexes = NULL;

        if (strcmp(word, "(") == 0) t->type = LEFT_PAR;
        else if (strcmp(word, ")") == 0) t->type = RIGHT_PAR;
        else if (strcmp(word, "not") == 0 && prev_not == 0) t->type = NOT;
        else if (strcmp(word, "or") == 0 && prev_op == 0) t->type = OR;
        else if (strcmp(word, "and") == 0 && prev_op == 0) t->type = AND;
        else {
            if (mode == 0)
                get_ids_from_word(word, &t->len, &t->indexes);
            else
                reduced_get_ids_from_word(word, &t->len, &t->indexes);
            t->type = TERM;
        }

        int current_op = t->type != TERM && t->type != RIGHT_PAR;

        if (current_op == prev_op && (t->type != NOT && t->type != LEFT_PAR && t->type != RIGHT_PAR)) {
            printf("Syntax error\n");
            return;
        }

        prev_op = current_op;
        prev_not = (t->type == NOT) ? 1 : 0;
        token_list[i++] = t;
        word = strtok(NULL, " \n");
    }
}

/**
 *
 * @param type Type of the token
 * @return For each token it return the priority for parsing the query
 */
int get_priority(enum Token_type type) {
    if (type == OR) return 1;
    if (type == AND) return 2;
    if (type == NOT) return 3;
    if (type == LEFT_PAR) return 4;
    if (type == RIGHT_PAR) return 4;
    return -1;
}

/**
 *
 * @param token_list Tokenize query
 * @param rpn_stack Query ordered in which operation go first
 * @param length Outer parameters: the size of the stack (size of tokenize query - all the parenthesis)
 * @details I used Reversed Polish Notation for get the order of which operation goes first
 * useful later for evaluating the query
 */
void create_rpn(token *token_list, Stack *rpn_stack, int *length) {
    Stack *op_stack = stack_create(*length);

    for (int i = 0; i < (*length); i++) {
        token t = token_list[i];
        if (t->type == TERM)
            stack_push(rpn_stack, t);
        else if (t->type == RIGHT_PAR) {
            while (op_stack->size != 0 && ((token) stack_last(op_stack))->type != LEFT_PAR) {
                stack_push(rpn_stack, stack_pop(op_stack));
            }
            free(stack_pop(op_stack));
            free(t);
        } else {
            while (op_stack->size != 0 && get_priority(t->type) < get_priority(((token) stack_last(op_stack))->type) &&
                   ((
                       token) stack_last(op_stack))->type != LEFT_PAR) {
                stack_push(rpn_stack, stack_pop(op_stack));
            }
            if (op_stack->size != 0 && get_priority(t->type) == get_priority(((token) stack_last(op_stack))->type)) {
                if (t->type != RIGHT_PAR)
                    stack_push(rpn_stack, stack_pop(op_stack));
                else
                    stack_pop(op_stack);
            }
            if (t->type != RIGHT_PAR)
                stack_push(op_stack, t);
        }
    }

    while (op_stack->size > 0)
        stack_push(rpn_stack, stack_pop(op_stack));

    *length = rpn_stack->size;
    stack_delete(op_stack);
}

/**
 *
 * @param len Outer parameter: how many ids are returned
 * @param a How many ids are in the input
 * @param idx_a Input ids
 * @return Array of ids not present in the input
 */
int *do_not(int *len, int a, int *idx_a) {
    if (ids_len - a == 0) {
        *len = 0;
        return NULL;
    }
    int *result = malloc(sizeof(int) * (ids_len - a));
    int ia = 0;
    int j = 0;
    for (int i = 0; i < ids_len; i++) {
        while (ia < a && all_ids[i] > idx_a[ia])
            ia++;
        if (ia >= a || all_ids[i] < idx_a[ia])
            result[j++] = all_ids[i];
    }
    *len = (ids_len - a);
    return result;
}

/**
 *
 * @param len Outer parameter: how many ids are returned
 * @param a How many ids are in the first input
 * @param idx_a First input ids
 * @param b How many ids are in the second input
 * @param idx_b Second input ids
 * @return Array of ids present in an input or in the other
 */
int *do_or(int *len, int a, int *idx_a, int b, int *idx_b) {
    int final_len = (a + b > ids_len) ? ids_len : a + b;
    if (final_len == 0) {
        *len = 0;
        return NULL;
    }
    int *result = malloc(sizeof(int) * (final_len));
    int ia = 0;
    int ib = 0;
    int i = 0;
    while (ia < a && ib < b) {
        if (idx_a[ia] < idx_b[ib])
            result[i++] = idx_a[ia++];
        else if (idx_a[ia] > idx_b[ib])
            result[i++] = idx_b[ib++];
        else
            result[i++] = idx_a[ia++] + 0 * ib++;
    }
    while (ia < a)
        result[i++] = idx_a[ia++];
    while (ib < b)
        result[i++] = idx_b[ib++];
    *len = i;
    if (i != 0)
        result = realloc(result, sizeof(int) * (i));
    else {
        free(result);
        result = NULL;
    }
    return result;
}

/**
 *
 * @param len Outer parameter: how many ids are returned
 * @param a How many ids are in the first input
 * @param idx_a First input ids
 * @param b How many ids are in the second input
 * @param idx_b Second input ids
 * @return Array of ids present in an input and in the other
 */
int *do_and(int *len, int a, int *idx_a, int b, int *idx_b) {
    int final_len = (a < b) ? b : a;
    if (final_len == 0) {
        *len = 0;
        return NULL;
    }
    int *result = malloc(sizeof(int) * (final_len));
    int ia = 0;
    int ib = 0;
    int i = 0;

    while (ia < a && ib < b) {
        if (idx_a[ia] < idx_b[ib])
            ia++;
        else if (idx_a[ia] > idx_b[ib])
            ib++;
        else
            result[i++] = idx_a[ia++] + 0 * ib++;
    }

    *len = i;
    if (i != 0)
        result = realloc(result, sizeof(int) * (i));
    else {
        free(result);
        result = NULL;
    }
    return result;
}

/**
 *
 * @param t token to be free
 */
void token_delete(token t) {
    free(t->indexes);
    free(t);
}

/**
 *
 * @param len Outer parameter: how many ids are after the evaluation
 * @param rpn_stack Query in the Reverse Polish Notation (RPN)
 * @return Ids after executing the query
 * @details It's a recursive function, it pops a token from the stack.
 * if it's a term it returns the ids and the length.
 * if it's a NOT operation it pops another token it evaluates it, then it perfroms the NOT operat.on.
 * if it's a OR/AND it pops one evaluates it, it pops another it evaluates it and then perform the operation.
 * In this way it should maintain the logic for the RPN
 */
int *evaluate(int *len, Stack *rpn_stack) {
    int *result = NULL;
    *len = 0;
    token t = stack_pop(rpn_stack);
    if (t == NULL) return NULL;
    if (t->type == TERM) {
        *len = t->len;
        result = malloc(sizeof(int) * (*len));
        for (int i = 0; i < *len; i++)
            result[i] = t->indexes[i];
        token_delete(t);
        return result;
    }
    int a;
    int *idx_a = evaluate(&a, rpn_stack);
    if (t->type == NOT) {
        result = do_not(len, a, idx_a);
        free(idx_a);
        token_delete(t);
        return result;
    }

    int b;
    int *idx_b = evaluate(&b, rpn_stack);
    if (t->type == OR)
        result = do_or(len, a, idx_a, b, idx_b);
    else if (t->type == AND)
        result = do_and(len, a, idx_a, b, idx_b);

    free(idx_a);
    free(idx_b);
    token_delete(t);
    return result;
}

/**
 *
 * @param indexes Outer parameter: Array of ids for the query
 * @param len Outer parameter: How many ids are gathered
 * @param q Query to parse
 * @details This is the full pipeline for parsing the query.
 * 1. tokenize the query
 * 2. order the query based on which operation comes first (RPN order)
 * 3. evaluate it
 */
void query_parser(int **indexes, int *len, char *q) {
    int size = get_number_of_words(q);
    token *token_list = malloc(sizeof(struct _token) * size);
    if (token_list == NULL)
        return;

    tokenize(q, token_list);

    Stack *rpn_stack = stack_create(size);
    if (rpn_stack == NULL) {
        free(token_list);
        return;
    }

    create_rpn(token_list, rpn_stack, &size);
    free(token_list);

    *indexes = evaluate(len, rpn_stack);

    stack_delete(rpn_stack);
}

/**
 *
 * @param m Wich alorithm it's gonna be used
 * @details m = 0 standard algorithm, m = 1 reduced algorithm
 */
void init_query_parser(int m) {
    if (m == 0)
        get_dict_and_ids(&dict, &all_ids, &ids_len);
    else
        reduced_get_dict_and_ids(&dict, &all_ids, &ids_len);
    mode = m;
}

/**
 *
 * @return The dictionary used for parsing the queries
 */
dictionary get_dict() {
    return dict;
}
