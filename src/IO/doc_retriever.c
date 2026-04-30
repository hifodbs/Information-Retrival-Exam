#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "IO/doc_retriever.h"

static char* trim_and_clean(char *str) {
    if (str == NULL) return NULL;
    
    // Trim leading whitespace
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;

    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    return str;
}

Document* get_document_by_id(const char *source_path, int id) {
    FILE *fp = fopen(source_path, "r");
    if (!fp) return NULL;

    char line[1024];
    int current_id = -1;
    Document *doc = NULL;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, ".I", 2) == 0) {
            current_id = atoi(line + 3);
            if (current_id == id) {
                doc = calloc(1, sizeof(Document));
                doc->id = id;
                
                char content[4096] = {0};
                int state = 0; // 0: looking for .T, 1: reading title, 2: reading abstract

                while (fgets(line, sizeof(line), fp)) {
                    if (strncmp(line, ".I", 2) == 0) break;
                    
                    if (strncmp(line, ".T", 2) == 0) {
                        state = 1;
                        content[0] = '\0';
                        continue;
                    } else if (strncmp(line, ".A", 2) == 0) {
                        if (state == 1) doc->title = strdup(trim_and_clean(content));
                        state = 0;
                        continue;
                    } else if (strncmp(line, ".W", 2) == 0) {
                        state = 2;
                        content[0] = '\0';
                        continue;
                    } else if (strncmp(line, ".B", 2) == 0) {
                        if (state == 2) doc->abstract = strdup(trim_and_clean(content));
                        state = 0;
                        continue;
                    }

                    if (state == 1 || state == 2) {
                        strncat(content, line, sizeof(content) - strlen(content) - 1);
                    }
                }
                
                // Fallback for end of file
                if (state == 1 && !doc->title) doc->title = strdup(trim_and_clean(content));
                if (state == 2 && !doc->abstract) doc->abstract = strdup(trim_and_clean(content));
                
                break;
            }
        }
    }

    fclose(fp);
    return doc;
}

void free_document(Document *doc) {
    if (doc) {
        free(doc->title);
        free(doc->abstract);
        free(doc);
    }
}
