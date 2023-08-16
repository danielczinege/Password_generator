#include "password_tools.h"
#include <openssl/rand.h>

#define CHAR_POOL_LENGTH ('~' - ' ' + 1)

/**
 * @note This function will ask continuously until user answers with "y" or "n".
 *
 * @param question What you want to ask. (Will be added " [y/n]" at the end of the question)
 * @param response Allocated memory with at least 3 chars. You must deallocate it yourself.
 * @param response_capacity Capacity of response.
 * @return true if user answered "y", false if they answered "n"
 */
bool yes_no_question(const char *question, char *response, int response_capacity)
{
    while (true) {
        printf("%s [y/n]\n", question);
        if (fgets(response, response_capacity, stdin) == NULL) {
            fprintf(stderr, "failed to read response\n");
            return false;
        }
        if (strlen(response) != 2) {
            continue;
        }
        if (response[0] == 'y') {
            return true;
        }
        if (response[0] == 'n') {
            return false;
        }
    }
}

/**
 *
 * @param response Has to be allocated memory. After failure you have to free it.
 * @param response_capacity Capacity of response.
 * @param length Pointer to length variable, where will be the desired length stored.
 * @return true if successful, false otherwise.
 */
bool get_password_length(char *response, int response_capacity, long *length)
{
    while (8 > *length || *length >= 1000) {
        printf("How long do you want your password to be?\n");

        if (fgets(response, response_capacity, stdin) == NULL) {
            fprintf(stderr, "failed to read response\n");
            return false;
        }

        char *end = response;
        *length = strtol(response, &end, 10);

        if (*length >= 1000 || 8 > *length || (*end != '\n' && *end != '\0')) {
            fprintf(stderr, "You should enter a number between 8 and 999, included.\n");
            *length = 0;
            continue;
        }

        if (8 <= *length && *length < 14) {
            printf("If you want to have a strong password I recommend having at least 14 characters.\n");
            if (yes_no_question("Would you like to change the password length to some higher number?", response, response_capacity)) {
                *length = 0;
            }
        }
    }
    return true;
}

/**
 *
 * @param response Has to be allocated memory. After failure you have to free it.
 * @param response_capacity Capacity of response.
 * @param character_pool Has to be allocated memory with length equal to CHAR_POOL_LENGTH = '~' - ' ' + 1.
 *                       To fit all the characters. After failure you have to free it.
 * @return true if successful, false otherwise.
 */
bool get_character_pool(char *response, int response_capacity, char *character_pool, int *char_pool_end_index)
{
    printf("\nWrite which characters you don't want. Characters that are automatically included are all ASCII characters,\n"
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
        return false;
    }

    for (char chr = ' '; chr <= (char) '~'; chr++) {
        character_pool[chr - ' '] = chr;
    }

    int left = 0;
    while (response[left] != '\n' && response[left] != '\0') {
        if (' ' <= response[left] && response[left] <= '~') {
            character_pool[response[left] - ' '] = '\1';
        }
        left++;
    }

    left = 0;
    int right = CHAR_POOL_LENGTH - 1;

    while (left < right) {
        if (character_pool[left] != '\1') {
            left++;
            continue;
        }

        while (left < right && character_pool[right] == '\1') {
            right--;
        }
        if (left == right) {
            break;
        }

        character_pool[left] = character_pool[right];
        character_pool[right] = '\1';
        right--;
        left++;
    }
    if (character_pool[left] == '\1') {
        left--;
    }
    *char_pool_end_index = left;
    return true;
}

/**
 * @return true if OpenSSL's random functions are initialized; false otherwise
 */
bool initialize_generator(bool *rand_initialized)
{
    if (*rand_initialized) {
        return true;
    }

    if (RAND_poll() == 0) {
        fprintf(stderr, "Error initializing OpenSSL.\n");
        return false;
    }

    if (RAND_status() == 0) {
        fprintf(stderr, "Insufficient entropy for secure random numbers.\n"
                        "Wait a bit before trying again.\n");
        return false;
    }

    *rand_initialized = true;
    return true;
}

bool generate_password(char *response, char *character_pool, int char_pool_end_index, long length, int response_capacity)
{
    unsigned char *random_bytes = malloc(length * sizeof(unsigned char));
    if (random_bytes == NULL) {
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    char *password = malloc((length + 1) * sizeof(char));
    if (password == NULL) {
        free(random_bytes);
        fprintf(stderr, "failed to allocate memory for password\n");
        return false;
    }

    bool safe_password = false;

    if (yes_no_question("\nWould you like to safe the generated password? (NOT ENCRYPTED)\n", response, response_capacity)) {
        safe_password = true;
    }

    if (! yes_no_question("\nYour password is going to be generated, make sure no one can see your password when it "
                          "will be displayed.\nWould you like to generate it right now?\n", response, response_capacity)) {
        free(random_bytes);
        free(password);
        return true;
    }

    if (RAND_bytes(random_bytes, length) != 1) {
        fprintf(stderr, "failed to generate random numbers\n");
        free(random_bytes);
        free(password);
        return false;
    }

    int char_pool_index = 0;
    char_pool_end_index++;

    for (int i = 0; i < length; i++) {
        char_pool_index = random_bytes[i] % char_pool_end_index;
        password[i] = character_pool[char_pool_index];
        random_bytes[i] = 0; //Just for safety
    }
    password[length] = '\0';

    printf("Your password is: %s\n", password);

    if (! safe_password) {
        memset(password, 0, length);
        free(password);
        free(random_bytes);
        return true;
    }

    return true;
}

bool generate_passwords(bool *rand_initialized)
{
    int response_capacity = MAX_CHAR_RANGE;
    char *response = malloc(response_capacity * sizeof(char));

    if (response == NULL) {
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    long length = 0;

    if (! get_password_length(response, response_capacity, &length)) {
        fprintf(stderr, "failed to get password length\n");
        free(response);
        return false;
    }

    char *character_pool = malloc( CHAR_POOL_LENGTH * sizeof(char));

    if (character_pool == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        free(response);
        return false;
    }

    int char_pool_end_index = 0;

    if (! get_character_pool(response, response_capacity, character_pool, &char_pool_end_index)) {
        free(response);
        free(character_pool);
        return false;
    }

    if (! initialize_generator(rand_initialized)) {
        free(response);
        free(character_pool);
        return false;
    }

    bool another_password = true;

    while (another_password) {
        if (! generate_password(response, character_pool, char_pool_end_index, length, response_capacity)) {
            free(response);
            free(character_pool);
            return false;
        }
        another_password = yes_no_question("Do you want to create another password with same length and character set as previous one?",
                                           response, response_capacity);
    }

    free(response);
    free(character_pool);
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
