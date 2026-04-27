#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>

#include "../../include/IO/one_timer_IO.h"
#include "../../include/utils/reverse_word.h"

char *stop_words[] = {".","(",")", 0};


/**
 *
 * @param fp Pointer to the file of Interest
 * @param stopWord Word when stop reading
 * @param lst Linked list where to add new words/ids
 * @param id The ids from what the words are gather from
 * @param stop Output parameter: if the function reach EOF it set stop to 0
 * @details Read words until it find parameter stopWord.
 * there is some word processing too, put all word lower case and
 * ignore stop_words
 */
void add_words(FILE *fp, char *stopWord, linked_list lst, int id, int *stop)
{
    char word[100];
    char *c;
    while (fscanf(fp, "%99s", word) != EOF)
    {
        if (!strcmp(word, stopWord))
            return;
        c = malloc(strlen(word) + 1);
        c[0] = '\0';
        strcat(c, word);
        for (size_t i = 0; i < strlen(c); i++)
            *(c + i) = tolower(*(c + i));
        int i = 0;
        bool stopword = false;
        while (stop_words[i])
        {
            if (strcmp(stop_words[i], c) == 0)
            {
                stopword = true;
                break;
            }
            i++;
        }
        if (!stopword)
            lst_add(lst, id, c);
        free(c);
    }
    *stop = 0;
}

/**
 *
* @param fp Pointer to the file of Interest
 * @param stopWord Word when stop reading
 * @return String of what it was read from fp
 * @details Used for read the Id of the docs
 */
char *get_string_until(FILE *fp, char *stopWord)
{
    char word[100];
    char *c = malloc(1);
    c[0] = '\0';
    while (fscanf(fp, "%99s", word) != EOF)
    {
        if (!strcmp(word, stopWord))
        {
            memmove(c, c + 1, strlen(c));
            for (size_t i = 0; i < strlen(c); i++)
                *(c + i) = tolower(*(c + i));
            return c;
        }
        c = realloc(c,strlen(c) + strlen(word) + 2); // Allocate enough memory for concatenation
        if (c == NULL)
        {
            printf("Memory allocation failed\n");
        }
        c[0] = '\0';     // Initialize the new memory as an empty string
        strcat(c, " ");  // Copy the old string into the new memory
        strcat(c, word); // Append the new string
    }
    memmove(c, c + 1, strlen(c));
    for (size_t i = 0; i < strlen(c); i++)
        *(c + i) = tolower(*(c + i));
    return c;
}

/**
 *
 * @param path Path to the source file
 * @return Linked list of a sparse inverted index
 */
linked_list get_words_and_idx(char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        printf("Unable to open the file\n");
        return NULL;
    }

    char word[100];
    int id;
    char *s_id;

    linked_list wordList = lst_make();

    int condition = 1;

    if (fscanf(fp, "%99s", word) == EOF)
        return NULL;

    while (condition)
    {
        s_id = get_string_until(fp, ".T");
        id = atoi(s_id);
        free(s_id);
        add_words(fp, ".A", wordList, id, &condition);
        add_words(fp, ".B", wordList, id, &condition);
        add_words(fp, ".W", wordList, id, &condition);
        add_words(fp, ".I", wordList, id, &condition);
    }

    fclose(fp);
    return wordList;
}

/**
 *
 * @param N Number of words
 * @param wordList Linked list of sparse words with associated ids
 * @param words Outer parameter: Array of words ordered alphabetically
 * @param idx Outer parameter: Array of ids for each word
 * @param size Outer parameter: How many ids for each word
 */
void sort_words(int N, linked_list wordList, char **words, int **idx, int *size)
{
    node target;
    for (int i = 0; i < N; i++)
    {
        target = lst_pop_first_alphabet(wordList);
        words[i] = target->word;
        idx[i] = target->id;
        size[i] = target->length;
        free(target);
    }
}

/**
 *
 * @param filename Path where to write the bin file
 * @param N How many words to write
 * @param words Array of words to write
 * @param idx Array of ids for each word
 * @param size How many ids for each word
 * @return 0 = OK, 1 = Error
 * @details create a binary file for each word it writes length of word,the word,how many ids have the word and the ids,
 * at the start of the file it writes how many words there will be
 */
int create_file(char *filename, int N, char **words, int **idx, int *size)
{
    remove(filename);

    FILE *fptr;
    // Create a file
    fptr = fopen(filename, "wb");
    if (fptr == NULL)
        return 1;


    fwrite(&N,sizeof(int),1,fptr);
    for (int i = 0; i < N; i++)
    {
        int length = strlen(words[i]);
        fwrite(&length,sizeof(int),1,fptr);
        fwrite(words[i],sizeof(char),length,fptr);
        fwrite(&size[i], sizeof(int), 1, fptr);
        fwrite(idx[i], sizeof(int), size[i], fptr);
    }

    // Close the file
    fclose(fptr);
    return 0;
}

/**
 *
 * @param source_file Path to the source file
 * @param bin_file Path where to write the bin file
 * @return 0 = OK, 1 = Error
 * @details full pipeline for creating a posting list
 */
int create_posting_list(char *source_file, char *bin_file) {
    linked_list word_list = NULL;
    word_list = get_words_and_idx(source_file);
    if (word_list == NULL)
        return 1;

    int N = lst_size(word_list);
    char *words[N];
    int *idx[N];
    int size[N];

    sort_words(N, word_list, words, idx, size);

    lst_delete(word_list);

    int return_status = 0;

    if (create_file(bin_file, N, words, idx, size)!=0)
        return_status = 1;

    for (int i = 0; i < N; i++) {
        free(words[i]);
        free(idx[i]);
    }
    return return_status;
}
