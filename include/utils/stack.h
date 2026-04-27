

#ifndef IR_STACK_H
#define IR_STACK_H

typedef struct  {
    void **data;
    int max_size;
    int size;
}Stack;

Stack *stack_create(int max);

void stack_push(Stack *s, void *p);

void *stack_pop(Stack *s);

void *stack_last(Stack *s);

void stack_delete(Stack *s);

void stack_print(Stack *s);


#endif //IR_STACK_H
