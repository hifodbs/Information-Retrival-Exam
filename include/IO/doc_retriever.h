#ifndef IR_DOC_RETRIEVER_H
#define IR_DOC_RETRIEVER_H

typedef struct {
    int id;
    char *title;
    char *abstract;
} Document;

Document* get_document_by_id(const char *source_path, int id);
void free_document(Document *doc);

#endif // IR_DOC_RETRIEVER_H
