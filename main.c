#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "password_tools.h"

int main(int argc, char *argv[])
{
    if (argc != 1) {
        fprintf(stderr, "Too many arguments.\n");
        return EXIT_FAILURE;
    }

    int response_capacity = 8;
    int response_length = 0;
    char *response = malloc(response_capacity * sizeof(char));

    if (response == NULL) {
        fprintf(stderr, "malloc failed\n");
        return EXIT_FAILURE;
    }

    bool rand_initialized = false;

    printf("This is a password generator, that can also estimate strength of your passwords or\n"
           "store your passwords in encrypted file, if you wish.\n");

    while (true) {
        printf("\nWhat do you want to do: (write the number)\n"
               "1 - generate passwords\n"
               "2 - check password strength\n"
               "3 - store passwords in encrypted file\n"
               "4 - get password from encrypted file\n"
               "5 - get help on making your own secure password\n"
               "6 - exit program\n");

        if (fgets(response, response_capacity, stdin) == NULL) {
            fprintf(stderr, "failed to read input\n");
            free(response);
            return EXIT_FAILURE;
        }

        if (response[0] == '\0' || (response[1] != '\n' && response[2] != '\0')) {
            fprintf(stderr, "_________________________________________\n"
                            "You must choose a number between 1 and 6.\n"
                            "_________________________________________\n");
            continue;
        }

        switch (response[0]) {
            case '1':
                //TODO call generation of passwords
                break;
            case '2':
                if (! password_strength()) {
                    free(response);
                    return EXIT_FAILURE;
                }
                break;
            case '3':
                //TODO call storing
                break;
            case '4':
                //TODO call getting
                break;
            case '5':
                printf("\nA strong password is one that's easy for you to remember but difficult for others to guess.\n"
                       "Here are some things to consider when creating your passwords:\n"
                       "- never use personal information and information that can be found on social media about you\n"
                       "- use longer passwords (at least 14 to 16 characters)\n"
                       "- a password should include a combination of letters, numbers, and characters\n"
                       "- a password shouldn’t be shared with any other account\n"
                       "- a password shouldn’t contain any consecutive letters or numbers\n"
                       "- random passwords are strongest\n"
                       "- you can make your password longer by adding smiles, like :), :(, =), :<, :S, ;), 8), :D, ...\n"
                       "\n"
                       "But how do you remember a strong password?\n"
                       "You can use a bizarre passphraze with symbols and numbers.\n"
                       "- don't choose dictionary words that typically go together\n"
                       "(e.g. 32 Seagulls deliver bologna sandwiches to Paris\n"
                       " or   32-Seagullsdeliver bologna5andwiches2Paris!)\n"
                       "\n"
                       "Or you can incorporate shortcuts or acronyms.\n"
                       "- Use phrases that mean something to you and shorten them by using shortcuts\n"
                       "(e.g 2BorNot2B_ThatisThe? (To be or not to be, that is the question-Shakespeare),\n"
                       " 1gbeFnw8f:)              (I go bowling every Friday night with 8 friends)\n"
                       "\n"
                       "Or you can use a password manager to manage your passwords, with this you can use completely random passwords\n");
                break;
            case '6':
                free(response);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "_________________________________________\n"
                                "You must choose a number between 1 and 6.\n"
                                "_________________________________________\n");
                continue;
        }
    }
}
