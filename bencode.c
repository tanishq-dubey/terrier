#include "bencode.h"

bencode_t* parse_bencode_file(char* filename) {
    // Open the file
    FILE* fp = fopen(filename, "r");
    // Return null if failure
    if (fp == NULL) {
        printf("Error opening the file\n");
        return NULL;
    }

    int i = 0, j = 1;
    bencode_t* bencode = (bencode_t*)malloc(sizeof(bencode_t));
    bencode->items = NULL;
    bencode->length = 0;
    while(fgetc(fp) != EOF) {
        fseek(fp, -1, SEEK_CUR);
        bencode_item_t* item = create_bencode_item(fp);
        if (item == NULL) {
            for (j = 0; j < i; j++) {
                free_bencode_item(bencode->items[j]);
            }
            return NULL;
        }
        bencode->items = (bencode_item_t**)realloc(bencode->items, j * sizeof(bencode_item_t*));
        bencode->items[i] = item;
        j++;
        i++;
        bencode->length++;
    }
    fclose(fp);
    return bencode;
}

void free_bencode(bencode_t* bencode) {
    int i;
    for(i = 0; i < bencode->length; i++) {
        free_bencode_item(bencode->items[i]);
    }
    free(bencode->items);
    free(bencode);
    bencode = NULL;
}

void print_bencode(bencode_t* bencode) {
    int i;
    for(i = 0; i < bencode->length; i++) {
        print_bencode_item(bencode->items[i], 0);
    }
}

bencode_item_t* create_bencode_item(FILE* fp) {
    char x = fgetc(fp);
    bencode_item_t* item = (bencode_item_t*)malloc(sizeof(bencode_item_t));
    switch(x) {
        // If it matches any of the types, begin processing for that type
        // else throw null return
        case 'i':
            item->item = parse_integer(fp);
            item->type = INTEGER;
            break;
        case 'l':
            // We encountered a list
            item->item = parse_list(fp);
            item->type = LIST;
            break;
        case 'd':
            // We encountered a dictionary
            // Parse the string
            item->item = parse_dict(fp);
            item->type = DICTIONARY;
            break;
        default:
            // We encountered a byte string (probably), check for ASCII value
            if (x > 47 && x < 58) {
                // Found int, parse
                // First move the pointer back one so that it can be correctly read:
                fseek(fp, -1, SEEK_CUR);
                item->item = parse_string(fp);
                item->type = BYTE_STRING;
            } else {
                // If it isn't a number, then it's bad
                free(item);
                printf("Invalid bencode format\n");
                return NULL;
            }
            break;
    }
    if (item->item == NULL) {
        free(item);
        return NULL;
    }
    return item;
}

void print_bencode_item(bencode_item_t* item, int print_depth) {
    int i;
    int j;
    for(i = 0; i < print_depth; i++){
        printf("\t");
    }
    switch(item->type) {
        case BYTE_STRING:
            printf("Byte String of length %d: %s\n", ((bencode_string_t*)item->item)->length,((bencode_string_t*)item->item)->string);
            break;
        case INTEGER:
            printf("Integer: %ld\n", ((bencode_integer_t*)item->item)->value);
            break;
        case LIST:
            printf("List of length: %d:\n", ((bencode_list_t*)item->item)->length);
            for(i = 0; i < ((bencode_list_t*)item->item)->length; i++) {
                print_bencode_item(((bencode_list_t*)item->item)->elements[i], print_depth+1);
            }
            break;
        case DICTIONARY:
            printf("Dictionary of length %d:\n", ((bencode_dict_t*)item->item)->length);
            for(i = 0; i < ((bencode_dict_t*)item->item)->length; i++) {
                for(j = 0; j < print_depth; j++){
                    printf("\t");
                }
                printf("\t");
                printf("%s:\n", ((bencode_dict_t*)item->item)->elements[i]->tag->string);
                print_bencode_item(((bencode_dict_t*)item->item)->elements[i]->element, print_depth+2);
            }
            break;
    }
}

void free_bencode_item(bencode_item_t* item) {
    int i;
    switch(item->type) {
        case BYTE_STRING:
            free(((bencode_string_t*)item->item)->string);
            free(item->item);
            break;
        case INTEGER:
            free(item->item);
            break;
        case LIST:
            for(i = 0; i < ((bencode_list_t*)item->item)->length; i++) {
                free_bencode_item(((bencode_list_t*)item->item)->elements[i]);
            }
            free(((bencode_list_t*)item->item)->elements);
            free(item->item);
            break;
        case DICTIONARY:
            for(i = 0; i < ((bencode_dict_t*)item->item)->length; i++) {
                free(((bencode_dict_t*)item->item)->elements[i]->tag->string);
                free(((bencode_dict_t*)item->item)->elements[i]->tag);
                free_bencode_item(((bencode_dict_t*)item->item)->elements[i]->element);
            }
            free(((bencode_dict_t*)item->item)->elements);
            free(item->item);
            break;
    }
    free(item);
    item = NULL;
}

bencode_string_t* parse_string(FILE* fp) {
    // Get byte string length
    char x;
    int i = 0;
    char val[1000];
    while (1) {
        x = fgetc(fp);
        if (x == ':') {
            break;
        }
        val[i] = x;
        i++;
    }
    val[i] = '\0';
    int len = strtol(val, NULL, 10);

    // Get the actual byte string
    bencode_string_t* strval = (bencode_string_t*)malloc(sizeof(bencode_integer_t));
    strval->string = (char*) malloc((len+2)*sizeof(char));
    if (fread(strval->string, 1, len, fp) != len) {
        free(strval->string);
        free(strval);
        printf("Invalid byte string parse in bencode\n");
        return NULL;
    }
    strval->string[len + 1] = '\0';
    strval->length = len;
    return strval;
}

bencode_integer_t* parse_integer(FILE* fp) {
    // Check for edge case of 0 value
    char x;
    char val[1000];
    fgets(val, 2, fp);
    if (val[0] == '0' && val[1] == 'e') {
        bencode_integer_t* intval = (bencode_integer_t*)malloc(sizeof(bencode_integer_t));
        intval->value = 0;
        return intval;
    } else if (val[0] == '0' && val[1] != 'e') {
        printf("Invalid integer parse in bencode\n");
        return NULL;
    }
    // We encountered a number
    fseek(fp, -1, SEEK_CUR);
    int i = 0;
    while(1) {
        x = fgetc(fp);
        if (x == 'e') {
            break;
        }
        val[i] = x;
        i++;
    }
    val[i] = '\0';
    bencode_integer_t* intval = (bencode_integer_t*)malloc(sizeof(bencode_integer_t));
    intval->value = strtol(val, NULL, 10);
    return intval;
}

bencode_list_t* parse_list(FILE* fp) {
    int i = 0, j = 1;
    bencode_list_t* list = (bencode_list_t*)malloc(sizeof(bencode_list_t));
    list->elements = NULL;
    list->length = 0;

    char x;
    while (1) {
        x = fgetc(fp);
        if (x == 'e') {
            break;
        }
        fseek(fp, -1, SEEK_CUR);
        bencode_item_t* tval = create_bencode_item(fp);
        if (tval != NULL) {
            list->elements = (bencode_item_t**)realloc(list->elements, j * sizeof(bencode_item_t*));
            list->elements[i] = tval;
            list->length++;
            i++;
            j++;
        } else {
            list->length = i;
            return list;
        }
    }
    return list;
}

bencode_dict_t* parse_dict(FILE* fp) {
    int i = 0, j = 1;
    bencode_dict_t* dict = (bencode_dict_t*)malloc(sizeof(bencode_dict_t));
    dict->elements = NULL;
    dict->length = 0;

    char x;
    while (1) {
        x = fgetc(fp);
        if (x == 'e') {
            break;
        }
        fseek(fp, -1, SEEK_CUR);

        bencode_string_t * d_string = parse_string(fp);
        if (d_string == NULL) {
            // Either the dictionary was empty, or the string parse was bad
            dict->length = i;
            return dict;
        }

        bencode_item_t * d_item = create_bencode_item(fp);
        if (d_item == NULL) {
            printf("Invalid dictionary item parse in bencode\n");
            return NULL;
        }
        bencode_dict_item_t* item = (bencode_dict_item_t*)malloc(sizeof(bencode_dict_item_t));
        item->tag = d_string;
        item->element = d_item;
        dict->elements = (bencode_dict_item_t**)realloc(dict->elements, j * sizeof(bencode_dict_item_t*));
        dict->elements[i] = item;
        dict->length++;
        i++;
        j++;
    }
    return dict;
}
