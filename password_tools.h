#ifndef PASSWORD_GENERATOR_PASSWORD_TOOLS_H
#define PASSWORD_GENERATOR_PASSWORD_TOOLS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#define LETTER_COUNT 26
#define DIGIT_COUNT 10
#define SPECIAL_CHARS 20
#define MAX_CHAR_RANGE (2 * LETTER_COUNT + DIGIT_COUNT + SPECIAL_CHARS)
#define MAX_PASSWORD_LENGTH 32

struct account_info {
    char *password;
    int password_length;

    char *account_name;
    int account_name_length;
};

struct site {
    char *name;
    int name_length;

    struct account_info *accounts;
    int account_count;
};

bool password_strength(void);
bool generate_passwords(bool *rand_initialized);
bool yes_no_question(const char *question, char *response, int response_capacity);

#endif //PASSWORD_GENERATOR_PASSWORD_TOOLS_H
