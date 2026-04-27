#include <stdio.h>
#include <stdlib.h>

#include "../../include/utils/stack.h"

/**
 *
 * @param max max size of the stack to be create
 * @return an allocated generic stack of pointer
 */
Stack *stack_create(int max) {
    Stack *s = malloc(sizeof(Stack));
    if (s == NULL)
        return NULL;
    s->max_size = max;
    s->data = malloc(sizeof(void *) * max);
    s->size = 0;
    return s;
}

/**
 *
 * @param s The stack to add a pointer
 * @param p Pointer to be added to the stack
 */
void stack_push(Stack *s, void *p) {
    if (s->size>=s->max_size) {
        fprintf(stderr, "Stack Overflow! Max size %d reached.\n", s->max_size);
        return;
    }
    s->data[s->size++] = p;
}

/**
 *
 * @param s The stack where to remove the last item inserted
 * @return The last item inserted
 */
void *stack_pop(Stack *s) {
    if (s->size == 0)
        return NULL;
    return s->data[--s->size];
}

/**
 *
 * @param s The stack to where to return the last item inserted without poping it
 * @return The last item inserted
 */
void *stack_last(Stack *s) {
    if (s->size == 0)
        return NULL;
    return s->data[s->size-1];
}

/**
 *
 * @param s Stack to be freed from memory
 * @details it frees the pointers too
 */
void stack_delete(Stack *s) {
    while (s->size>0) {
        void *p = stack_pop(s);
        free(p);
    }
    free(s->data);
    free(s);
}


