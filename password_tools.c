#include "password_tools.h"

bool password_strength(void)
{
    bool upper = false;
    bool lower = false;
    bool digits = false;
    bool special = false;
    int char_range = 0;

    char *password = malloc(MAX_PASSWORD_LENGTH * sizeof(char));
    if (password == NULL) {
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    if (fgets(password, MAX_PASSWORD_LENGTH, stdin) == NULL) {
        fprintf(stderr, "failed to read password\n");
        memset(password, 0, MAX_PASSWORD_LENGTH);
        free(password);
        return false;
    }

    printf("Your password is ");

    size_t length = strlen(password);
    if (length == MAX_PASSWORD_LENGTH - 1) {
        printf("very strong.\n");
        memset(password, 0, MAX_PASSWORD_LENGTH);
        free(password);
        return true;
    }
}
