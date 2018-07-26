#ifndef BENCODE_H
#define BENCODE_H

#include <stdio.h>
#include <stdlib.h>

enum bencode_type {
    BYTE_STRING,
    INTEGER,
    LIST,
    DICTIONARY
};

typedef struct bencode_item {
    enum bencode_type type;
    void* item;
} bencode_item_t;

typedef struct bencode_string {
    char* string;
    int length;
} bencode_string_t;

typedef struct bencode_integer {
    long value;
} bencode_integer_t;

typedef struct bencode_list {
    bencode_item_t** elements;
    int length;
} bencode_list_t;

typedef struct bencode_dict_item {
    bencode_string_t* tag;
    bencode_item_t* element;
} bencode_dict_item_t;

typedef struct bencode_dict {
    bencode_dict_item_t** elements;
    int length;
} bencode_dict_t;

typedef struct bencode {
    bencode_item_t** items;
    int length;
} bencode_t;

bencode_t* parse_bencode_file(char* filename);

bencode_item_t* create_bencode_item(FILE* fp);
void free_bencode(bencode_t* bencode);
void free_bencode_item(bencode_item_t* item);


void print_bencode(bencode_t* bencode);
void print_bencode_item(bencode_item_t* item, int print_depth);

bencode_string_t* parse_string(FILE* fp);
bencode_integer_t* parse_integer(FILE * fp);
bencode_list_t* parse_list(FILE* fp);
bencode_dict_t* parse_dict(FILE* fp);

#endif /* BENCODE_H */
