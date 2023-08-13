#include "password_tools.h"

bool generate_passwords(bool *rand_initialized)
{
    int response_capacity = MAX_CHAR_RANGE;
    char *response = malloc(response_capacity * sizeof(char));

    if (response == NULL) {
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    long length = 0;

    while (8 >= length || length >= 1000) {
        printf("How long do you want your password to be?\n");

        if (fgets(response, response_capacity, stdin) == NULL) {
            fprintf(stderr, "failed to read response\n");
            free(response);
            return false;
        }

        char *end = response;
        length = strtol(response, &end, 10);

        if (length >= 1000 || 8 >= length || (*end != '\n' && *end != '\0')) {
            fprintf(stderr, "You should enter a number between 1 and 999, included.");
        }

        if (8 < length && length < 12) {
            printf("If you want to have a strong password I recommend having at least 14 characters.\n");
            bool change_password = false;
            while (true) {
                printf("Would you like to change the password length to some higher number? [y/n]\n");
                if (fgets(response, response_capacity, stdin) == NULL) {
                    fprintf(stderr, "failed to read response\n");
                    free(response);
                    return false;
                }
                if (strlen(response) != 2) {
                    continue;
                }
                if (response[0] == 'y') {
                    change_password = true;
                    break;
                }
                if (response[0] == 'n') {
                    break;
                }
            }
            if (change_password) {
                continue;
            }
        }
    }

    printf("Write which characters you don't want. Characters that are automatically included are all ASCII characters,\n"
           "those are all upper and lower case letters, all digits, space and these special characters:\n");
    for (int chr = '!'; chr <= '/'; chr++) {
        putchar(chr);
    }
    for (int chr = ':'; chr <= '@'; chr++) {
        putchar(chr);
    }
    for (int chr = '['; chr <= '`'; chr++) {
        putchar(chr);
    }
    for (int chr = '{'; chr <= '~'; chr++) {
        putchar(chr);
    }
    putchar('\n');

    printf("Write the characters you don't want in your password one after another, like this: s.,5;~A\n");

    if (fgets(response, response_capacity, stdin) == NULL) {
        fprintf(stderr, "failed to read response\n");
        free(response);
        return false;
    }

    int char_pool_length = '~' - ' ' + 1;
    char *character_pool = malloc( char_pool_length * sizeof(char));
    if (character_pool == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        free(response);
        return false;
    }

    for (char chr = ' '; chr <= (char) '~'; chr++) {
        character_pool[chr - ' '] = chr;
    }

    int left = 0;
    while (response[left] != '\n' && response[left] != '\0') {
        if (' ' <= response[left] && response[left] <= '~') {
            character_pool[response[left] - ' '] = '\0';
        }
        left++;
    }

    left = 0;
    int right = char_pool_length - 1;

    while (left < right) {
        if (character_pool[left] != '\0') {
            left++;
            continue;
        }

        while (left < right && character_pool[right] == '\0') {
            right--;
        }
        if (left == right) {
            break;
        }

        character_pool[left] = character_pool[right];
        character_pool[right] = '\0';
        right--;
        left++;
    }
    if (character_pool[left] == '\0') {
        left--;
    }
    //TODO - debug this
    return true;
}

/**
 * @brief Asks for a password, calculates its entropy and uses it to tell the strength of the password.
 *        The strength is written in bold.
 * @note After entropy calculation the password is overwritten and the memory is freed.
 * @return true on success, false on failure
 */
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
            password[i] = '\0';
            continue;
        }

        if (islower(password[i])) {
            if (! lower) {
                lower = true;
                char_range += LETTER_COUNT;
            }
            password[i] = '\0';
            continue;
        }

        if (isdigit(password[i])) {
            if (! digits) {
                digits = true;
                char_range += DIGIT_COUNT;
            }
            password[i] = '\0';
            continue;
        }

        if (! special) {
            special = true;
            char_range += SPECIAL_CHARS;
        }
        password[i] = '\0';
    }

    memset(password, 0, MAX_PASSWORD_LENGTH);
    free(password);

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
    return true;
}
