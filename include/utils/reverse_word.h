#ifndef REVERSE_WORD_H
#define REVERSE_WORD_H

#include <stdbool.h>

struct _node
{
    int *id;
    int length;
    char *word;
    struct _node *next;
};

typedef struct _node *node;

struct _linked_list {
  node head;
};

typedef struct _linked_list * linked_list;

linked_list lst_make(void);

/* Elimina la lista e tutto il suo contenuto */
void lst_delete(linked_list lst);

/* Aggiunge alla testa della lista un nuovo nodo con la chiave indicata */
void lst_add(linked_list lst, int id, char *word);

/* Ritorna vero se e solo se il valore cercato è presente nella lista */
bool lst_search(linked_list lst, char *word);


void lst_print(linked_list lst);

int lst_size(linked_list lst);

node lst_pop_first_alphabet(linked_list lst);

#endif // REVERSE_WORD_H