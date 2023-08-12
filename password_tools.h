#ifndef PASSWORD_GENERATOR_PASSWORD_TOOLS_H
#define PASSWORD_GENERATOR_PASSWORD_TOOLS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define LETTER_COUNT 26
#define DIGIT_COUNT 10
#define SPECIAL_CHARS 20
#define MAX_CHAR_RANGE (2 * LETTER_COUNT + DIGIT_COUNT + SPECIAL_CHARS)
#define MAX_PASSWORD_LENGTH 32

bool password_strength(void);

#endif //PASSWORD_GENERATOR_PASSWORD_TOOLS_H
