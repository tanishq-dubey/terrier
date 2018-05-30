#include "bencode.h"


bencode_item_t* parse_bencode_file(char* filename) {
    // Open the file
    FILE* fp = fopen(filename, "r");
    // Return null if failure
    if (fp == NULL) {
        printf("Error opening the file");
        return NULL;
    }
	fclose(fp);
}

bencode_item_t* create_bencode_item(FILE* fp) {
    char x = fgetc(fp);
    char val[1000];
    switch(x) {
        // If it matches any of the types, begin processing for that type
        // else throw null return
        case 'i':
            // Check for edge case of 0 value
            fgets(val, 2, fp);
            if (val[0] == '0' && val[1] == 'e') {
                bencode_item_t* retval = (bencode_item_t*)malloc(sizeof(bencode_item_t));
                bencode_integer_t* intval = (bencode_integer_t*)malloc(sizeof(bencode_integer_t));
                retval->type = INTEGER;
                retval->item = intval;
                intval->value = 0;
                return retval;
            } else if (val[0] == '0' && val[1] != 'e') {
                printf("Invalid integer parse in bencode");
                return NULL;
            }
            // We encountered a number
            fseek(fp, -2, SEEK_CUR);
            int i = 0;
            while(x != 'e') {
                x = fgetc(fp);
                val[i] = x;
                i++;
            }
            val[i] = '\0';
            bencode_item_t* retval = (bencode_item_t*)malloc(sizeof(bencode_item_t));
            bencode_integer_t* intval = (bencode_integer_t*)malloc(sizeof(bencode_integer_t));
            retval->type = INTEGER;
            retval->item = intval;
            intval->value = atoi(val);
            return retval;
        case 'l':
            // We encountered a list
            break;
        case 'd':
            // We encountered a dictionary
            break;
        default:
            // We encountered a byte string (probably), check for ASCII value
            if (x > 47 && x < 58) {
				// Get byte string length
                int i = 0;
                while (x != ':') {
                    val[i] = x;
                    i++;
                    x = fgetc(fp);
                }
                val[i] = '\0';
				int len = atoi(val);

				// Get the actual byte string
				if (fgets(val, len, fp) == NULL) {
					printf("Invalid byte string parse in bencode");
					return NULL;
				}
				bencode_item_t* retval = (bencode_item_t*)malloc(sizeof(bencode_item_t));
				bencode_string_t* strval = (bencode_integer_t*)malloc(sizeof(bencode_integer_t));
				retval->type = BYTE_STRING;
				retval->item = strval;
				strval->value = val;
				strval->length = len;
				return retval;
            } else {
                // If it isn't a number, then it's bad
                printf("Invalid bencode format");
                return NULL;
            }
            break;
    }
}
