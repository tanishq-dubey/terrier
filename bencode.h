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

typedef struct bencode_string {
    char* string;
    int length;
} bencode_string_t;

typedef struct bencode_integer {
    int value;
} bencode_integer_t;

typedef struct bencode_list {
    void* elements[];
} bencode_list_t;

typedef struct bencode_dict_item {
    char* tag;
    void * element;
} bencode_dict_item_t;

typedef struct bencode_dict {
    bencode_dict_item_t* elements[];
} bencode_dict_t;

typedef struct bencode_item {
    bencode_type type;
    void* item;
} bencode_item_t;

bencode_item_t* parse_bencode_file(char* filename);

#endif /* BENCODE_H */
