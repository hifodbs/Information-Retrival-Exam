#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../include/utils/reverse_word.h"

/**
 *
 * @return Create a linked list
 */
linked_list lst_make()
{
    linked_list lst = (linked_list)malloc(sizeof(struct _linked_list));
    lst->head = NULL;
    return lst;
}

/**
 *
 * @param lst the linked list to free from memory
 */
void lst_delete(linked_list lst)
{
    if (lst == NULL)
    {
        return;
    }
    node current = lst->head;
    while (current != NULL)
    {
        node prev = current;
        current = prev->next;
        free(prev->id);
        free(prev->word);
        free(prev);
    }
    free(lst);
}

/**
 *
 * @param current Node of a linked list
 * @param id Id to be added at the linked list
 * @details the id is add only if it's not already present
 * and if it's added it will be in a order way
 */
void add_id_if_necessary(node current, int id)
{
    for (int i = 0; i < current->length; i++)
        if (current->id[i] == id)
            return;

    current->id = realloc(current->id, sizeof(int) * (current->length + 1));
    current->length += 1;

    int prev = id;
    int temp;
    for (int i = 0; i < current->length-1;i++) {
        if (current->id[i]>prev) {
            temp = prev;
            prev = current->id[i];
            current->id[i] = temp;
        }
    }
    current->id[current->length-1] = prev;
}

/**
 *
 * @param lst linked list where to put the word
 * @param id the docId associated with the word
 * @param word word to be inserted in the array list
 * @details if the word is already presente check if it is necessary to add the id
 */
void lst_add(linked_list lst, int id, char *word)
{
    if (lst == NULL)
        return;

    node current = lst->head;
    node prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->word, word) == 0)
        {
            add_id_if_necessary(current, id);
            return;
        }
        prev = current;
        current = current->next;
    }

    node new = (node)malloc(sizeof(struct _node));
    new->id = malloc(sizeof(int));
    new->id[0] = id;
    new->length = 1;
    new->word = strdup(word);
    new->next = NULL;
    if (prev == NULL)
        lst->head = new;
    else
        prev->next = new;
}

/**
 *
 * @param lst linked list where to search
 * @param word word to be search
 * @return true if the word exist false otherwise
 */
bool lst_search(linked_list lst, char *word)
{
    if (lst == NULL)
        return false;

    node current = lst->head;
    while (current != NULL)
    {
        if (!strcmp(current->word, word))
            return true;

        current = current->next;
    }
    return false;
}

/**
 *
 * @param lst linked list to gather the size
 * @return number of nodes in the linked list
 */
int lst_size(linked_list lst)
{
    if (lst == NULL)
        return -1;
    int size = 0;
    node current = lst->head;
    while (current != NULL)
    {
        size++;
        current = current->next;
    }
    return size;
}

/**
 *
 * @param lst linked list where to pop the node
 * @return the first node in a alphabetical order
 * @attention Destructive function, it remove the node from the linked list
 */
node lst_pop_first_alphabet(linked_list lst)
{
    if (lst == NULL || lst->head == NULL)
        return NULL;

    node current = lst->head;
    node target = current;
    node prev = NULL;
    node prev_target = NULL;

    while (current != NULL)
    {
        if (strcmp(current->word, target->word) < 0)
        {
            prev_target = prev;
            target = current;
        }
        prev = current;
        current = current->next;
    }
    if (prev_target == NULL)
        lst->head = target->next;
    else
        prev_target->next = target->next;
    return target;
}

/**
 *
 * @param lst linked list to be printed
 */
void lst_print(linked_list lst)
{
    if (lst == NULL)
    {
        printf("NIL");
        return;
    }
    printf("(");
    node current = lst->head;
    while (current != NULL)
    {
        printf("%s", current->word);
        for (int i = 0; i < current->length; i++)
            printf(" %d", current->id[i]);
        printf("\n");
        if (current->next != NULL)
        {
            printf(" ");
        }
        current = current->next;
    }
    printf(")\n");
}