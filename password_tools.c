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

    printf("Enter your password (it will be deleted immediately after the strength test):");

    if (fgets(password, MAX_PASSWORD_LENGTH, stdin) == NULL) {
        fprintf(stderr, "failed to read password\n");
        memset(password, 0, MAX_PASSWORD_LENGTH);
        free(password);
        return false;
    }

    printf("\nYour password is ");

    size_t length = strlen(password);
    if (password[length - 1] == '\n') {
        length--;
        password[length] = '\0';
    }

    printf("\e[1m");
    if (length == MAX_PASSWORD_LENGTH - 1) {
        printf("very strong");
        memset(password, 0, MAX_PASSWORD_LENGTH);
        free(password);
        return true;
    }

    for (int i = 0; i < length; i++) {
        if (char_range == MAX_CHAR_RANGE) {
            break;
        }

        if (isupper(password[i])) {
            if (! upper) {
                upper = true;
                char_range += LETTER_COUNT;
            }
            continue;
        }

        if (islower(password[i])) {
            if (! lower) {
                lower = true;
                char_range += LETTER_COUNT;
            }
            continue;
        }

        if (isdigit(password[i])) {
            if (! digits) {
                digits = true;
                char_range += DIGIT_COUNT;
            }
            continue;
        }

        if (! special) {
            special = true;
            char_range += SPECIAL_CHARS;
        }
    }

    double password_entropy = length * log2(char_range);

    if (password_entropy < 25) {
        printf("very weak");
    } else if (password_entropy < 50) {
        printf("weak");
    } else if (password_entropy < 75) {
        printf("reasonable");
    } else if (password_entropy < 100) {
        printf("strong");
    } else {
        printf("very strong");
    }

    printf("\e[m.\n");

    memset(password, 0, MAX_PASSWORD_LENGTH);
    free(password);
    return true;
}
