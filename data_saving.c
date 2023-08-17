#include "data_saving.h"
#include <stdio.h>
#include <string.h>

//Because I allow passwords to be 999 characters long
#define MAX_EXPECTED_LINE_LENGTH 1000

const char *data_file = "file";
const char *aux_file = "aux";

bool save_password(char *site_name, int name_length, struct account_info account)
{
    char buffer[MAX_EXPECTED_LINE_LENGTH + 1];

    FILE *file = fopen(data_file, "r");
    if (file == NULL) {
        fprintf(stderr, "failed to open file with data\n");
        return false;
    }

    while (fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) != NULL) {
        if (buffer[name_length] != '\n') {
            fprintf(stderr, "data file was modified\n");
            fclose(file);
            return false;
        }
        buffer[name_length] = '\0';
        if (strcmp(buffer, site_name) != 0) {
            //TODO - skip to another site
        }
        //TODO - add account
    }
}
