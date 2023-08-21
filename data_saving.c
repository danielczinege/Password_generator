#include "data_saving.h"
#include "password_tools.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

//Because I allow passwords to be 999 characters long
#define MAX_EXPECTED_LINE_LENGTH 1000

const char *data_file = "file";
const char *aux_file = "aux";

/**
 * @note Reads accounts and saves them to accounts until account_name is found.\n
 * You must have read all sites until the site where it is supposed to be and also the account_count.
 *
 * @param account_name Account to be found. (after the name there should be end of line character)
 * @param account_count Number of accounts saved for this site.
 * @param accounts Allocated array, where will be stored all read accounts until account_name was reached (not including it).
 * @param buffer Must be at least 1000 chars long
 * @param file Opened file with data
 * @param found_account is set to true if the account was found or to false if it wasn't found
 * @param loaded_count number of structs in an array accounts that have to be freed later.
 * @return true if no error occurs, false otherwise
 */
bool load_until_account(char *account_name, long account_count, struct account_info **accounts,
                        char *buffer, FILE *file, bool *found_account, int *loaded_count)
{
    *loaded_count = 0;

    for (long i = 0; i < account_count; i++) {
        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
            fprintf(stderr, "failed to read a line - data file was probably altered\n");
            return false;
        }

        if (strcmp(buffer, account_name) == 0) {
            if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
                fprintf(stderr, "failed to read a line - data file was probably altered\n");
                return false;
            }
            *found_account = true;
            return true;
        }

        struct account_info *account = malloc(sizeof(*account));
        if (account == NULL) {
            fprintf(stderr, "malloc failed\n");
            return false;
        }

        account->account_name_length = strlen(buffer);

        account->account_name = malloc((account->account_name_length + 1) * sizeof(char));
        if (account->account_name == NULL) {
            free(account);
            fprintf(stderr, "malloc failed\n");
            return false;
        }

        memcpy(account->account_name, buffer, account->account_name_length + 1);

        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
            fprintf(stderr, "failed to read a line - data file was probably altered\n");
            free(account->account_name);
            free(account);
            return false;
        }

        account->password_length = strlen(buffer);
        account->password = malloc((account->password_length + 1) * sizeof(char));
        if (account->password == NULL) {
            fprintf(stderr, "malloc failed\n");
            free(account->account_name);
            free(account);
            return false;
        }
        memcpy(account->password, buffer, account->password_length + 1);
        accounts[*loaded_count] = account;
        *loaded_count += 1;
    }
    *found_account = false;
    return true;
}

/**
 * @note frees all accounts and their attributes and also the accounts array
 */
void free_accounts(struct account_info **accounts, int loaded_count)
{
    for (int i = 0; i < loaded_count; i++) {
        free(accounts[i]->account_name);
        memset(accounts[i]->password, '\0', accounts[i]->password_length);
        free(accounts[i]->password);
        free(accounts[i]);
    }
    free(accounts);
}

/**
 * @param accounts accounts to be saved to the write file
 * @param loaded_count number of accounts in accounts
 * @param write opened file for writing
 */
void write_loaded_accounts(struct account_info **accounts, int loaded_count, FILE *write)
{
    for (int i = 0; i < loaded_count; i++) {
        fprintf(write, "%s", accounts[i]->account_name);
        fprintf(write, "%s", accounts[i]->password);
    }
}

/**
 * @note reads lines from <file> until all accounts from current site are read and rewrites them to <write>.
 *
 * @param file file with data
 * @param write file where we write data (if it is NULL then just skips to another site and does not print anything)
 * @param buffer at least 1000 characters long
 * @return true if no error occurs, false otherwise
 */
bool skip_to_another_site(FILE *file, FILE *write, char *buffer)
{
    if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
        fprintf(stderr, "failed to read a line - data file was probably altered\n");
        return false;
    }

    long count = strtol(buffer, NULL, 10);
    if (0 >= count || errno == ERANGE) {
        fprintf(stderr, "data file was probably altered\n");
        return false;
    }

    if (write != NULL) {
        fprintf(write, "%ld\n", count);
    }

    count *= 2;

    while (count > 0 && fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) != NULL) {
        if (write != NULL) {
            fprintf(write, "%s", buffer);
        }
        count--;
    }

    return count == 0;
}

/**
 * @note if account.password == NULL, then the function deletes the account\n
 * if you want to save password for account that is already there the password will change to the new one
 *
 * @param site_name name of a site, where the account is
 * @param account information about the account to be saved
 * @return true if no error occurred, false otherwise
 */
bool save_or_delete_password(char *site_name, struct account_info *account)
{
    char buffer[MAX_EXPECTED_LINE_LENGTH + 1];

    FILE *file = fopen(data_file, "a");
    if (file == NULL) {
        fprintf(stderr, "failed to open file with data\n");
        return false;
    }

    fclose(file);

    file = fopen(data_file, "r");
    if (file == NULL) {
        fprintf(stderr, "failed to open file with data\n");
        return false;
    }

    FILE *write = fopen(aux_file, "w");
    if (write == NULL) {
        fclose(file);
        fprintf(stderr, "failed to open file with data\n");
        return false;
    }

    while (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) != NULL) {
        if (strcmp(buffer, site_name) != 0) {
            fprintf(write, "%s", buffer);
            if (! skip_to_another_site(file, write, buffer)) {
                fclose(file);
                fclose(write);
                return false;
            }
            continue;
        }

        size_t length = strlen(buffer) + 1;
        char *name = malloc(length * sizeof(char));
        if (name == NULL) {
            fclose(file);
            fclose(write);
            fprintf(stderr,"malloc failed\n");
            return false;
        }

        memcpy(name, buffer, length);

        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
            free(name);
            fclose(file);
            fclose(write);
            fprintf(stderr, "failed to read a line - data file was probably altered\n");
            return false;
        }

        long count = strtol(buffer, NULL, 10);
        if (0 >= count || errno == ERANGE) {
            fprintf(stderr, "data file was probably altered\n");
            free(name);
            fclose(write);
            fclose(file);
            return false;
        }

        struct account_info **accounts = malloc(count * sizeof(*accounts));
        if (accounts == NULL) {
            fprintf(stderr, "malloc failed\n");
            free(name);
            fclose(write);
            fclose(file);
            return false;
        }

        int loaded_count = 0;
        bool found_account = false;

        if (! load_until_account(account->account_name, count, accounts, buffer, file, &found_account, &loaded_count)) {
            free_accounts(accounts, loaded_count);
            free(name);
            fclose(file);
            fclose(write);
            return false;
        }

        if (account->password == NULL) {
            if (! found_account) {
                fprintf(stderr, "The account was not found.\n");
                fprintf(write, "%s", name);
                fprintf(write, "%ld\n", count);
            } else if (count > 1) {
                fprintf(write, "%s", name);
                fprintf(write, "%ld\n", count - 1);
            }
        } else if (found_account) {
            fprintf(write, "%s", name);
            fprintf(write, "%ld\n", count);
        } else {
            fprintf(write, "%s", name);
            fprintf(write, "%ld\n", count + 1);
        }

        free(name);

        write_loaded_accounts(accounts, loaded_count, write);
        free_accounts(accounts, loaded_count);

        if (account->password != NULL) {
            fprintf(write, "%s", account->account_name);
            fprintf(write, "%s", account->password);
        }

        while (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) != NULL) {
            fprintf(write, "%s", buffer);
        }

        if (feof(file)) {
            fclose(file);
            fclose(write);

            if (remove(data_file)) {
                fprintf(stderr, "failed to change data_file\n");
                return false;
            }

            if (rename(aux_file, data_file)) {
                fprintf(stderr, "failed to change data_file\n");
                return false;
            }
            return true;
        }

        fclose(file);
        fclose(write);

        fprintf(stderr, "failed to read a line - data file was probably altered\n");
        return false;
    }

    if (! feof(file)) {
        fclose(file);
        fclose(write);
        fprintf(stderr, "failed to read a line\n");
        return false;
    }

    fclose(file);
    if (account->password == NULL) {
        fprintf(stderr, "The account was not found\n");
        fclose(write);
        if (remove(aux_file)) {
            fprintf(stderr, "failed to change data_file\n");
            return false;
        }
        return true;
    }

    fprintf(write, "%s", site_name);
    fprintf(write, "%c\n", '1');
    fprintf(write,"%s%s", account->account_name, account->password);

    fclose(write);

    if (remove(data_file)) {
        fprintf(stderr, "failed to change data_file\n");
        return false;
    }

    if (rename(aux_file, data_file)) {
        fprintf(stderr, "failed to change data_file\n");
        return false;
    }

    return true;
}

/**
 * @note Asks for account info and calls save_or_delete_password
 *
 * @return true if successful, false otherwise
 */
bool get_and_remove_password(void)
{
    char *site_name = malloc((LONGEST_NAME + 1) * sizeof(char));
    if (site_name == NULL) {
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    char *account_name = malloc((LONGEST_NAME + 1) * sizeof(char));

    if (account_name == NULL) {
        free(site_name);
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    struct account_info *account = malloc(sizeof(*account));
    if (account == NULL) {
        free(site_name);
        free(account_name);
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    account->account_name = account_name;
    account->password = NULL;

    printf("Please write which account data you want to delete:\n");

    if (fgets(account->account_name, LONGEST_NAME + 1, stdin) == NULL) {
        free(site_name);
        free(account_name);
        free(account);
        fprintf(stderr, "failed to read input\n");
        return false;
    }

    printf("Please write to what site is this account:\n");

    if (fgets(site_name, LONGEST_NAME + 1, stdin) == NULL) {
        free(site_name);
        free(account_name);
        free(account);
        fprintf(stderr, "failed to read input\n");
        return false;
    }

    if (! save_or_delete_password(site_name, account)) {
        free(site_name);
        free(account_name);
        free(account);
        return false;
    }

    free(site_name);
    free(account_name);
    free(account);
    return true;
}

/**
 * @note Asks for account info and calls save_or_delete_password
 *
 * @return true if successful, false otherwise
 */
bool get_and_save_password(void)
{
    char *site_name = malloc((LONGEST_NAME + 1) * sizeof(char));
    if (site_name == NULL) {
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    char *account_name = malloc((LONGEST_NAME + 1) * sizeof(char));

    if (account_name == NULL) {
        free(site_name);
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    struct account_info *account = malloc(sizeof(*account));
    if (account == NULL) {
        free(site_name);
        free(account_name);
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    account->account_name = account_name;

    account->password = malloc((MAX_EXPECTED_LINE_LENGTH + 1) * sizeof(char));
    if (account->password == NULL) {
        free(site_name);
        free(account_name);
        free(account);
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    printf("Write the password that you want to save:\n");

    if (fgets(account->password, MAX_EXPECTED_LINE_LENGTH + 1, stdin) == NULL) {
        free(account->password);
        free(site_name);
        free(account_name);
        free(account);
        fprintf(stderr, "failed to read input\n");
        return false;
    }

    account->password_length = strlen(account->password);

    if (account->password_length == MAX_EXPECTED_LINE_LENGTH && account->password[MAX_EXPECTED_LINE_LENGTH - 1] != '\n') {
        memset(account->password, 0, account->password_length);
        free(account->password);
        free(site_name);
        free(account_name);
        free(account);
        fprintf(stderr, "The password is too long.\n");
        return false;
    }

    printf("Please write to what site is this password: (you can write what you want here, it's just for you so that you can retrieve this password later)\n");

    if (fgets(site_name, LONGEST_NAME + 1, stdin) == NULL) {
        memset(account->password, 0, account->password_length);
        free(account->password);
        free(site_name);
        free(account_name);
        free(account);
        fprintf(stderr, "failed to read input\n");
        return false;
    }

    printf("And please write to what account is this password:\n");

    if (fgets(account->account_name, LONGEST_NAME + 1, stdin) == NULL) {
        memset(account->password, 0, account->password_length);
        free(account->password);
        free(site_name);
        free(account_name);
        free(account);
        fprintf(stderr, "failed to read input\n");
        return false;
    }

    if (! save_or_delete_password(site_name, account)) {
        memset(account->password, 0, account->password_length);
        free(account->password);
        free(site_name);
        free(account_name);
        free(account);
        return false;
    }

    memset(account->password, 0, account->password_length);
    free(account->password);
    free(site_name);
    free(account_name);
    free(account);
    return true;
}

/**
 * @note reads lines from <file> until all accounts from current site are read and writes them in formatted form to stdin.
 *
 * @param file file with data
 * @param buffer at least 1000 characters long
 * @return true if no error occurs, false otherwise
 */
bool print_site(char *buffer, FILE *file)
{
    if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
        fprintf(stderr, "failed to read a line - data file was probably altered\n");
        return false;
    }

    long count = strtol(buffer, NULL, 10);
    if (0 >= count || errno == ERANGE) {
        fprintf(stderr, "data file was probably altered\n");
        return false;
    }

    count *= 2;

    while (count > 0 && fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) != NULL) {
        if (count % 2 == 0) {
            printf("    Account name: %s", buffer);
        } else {
            printf("    Password: %s\n", buffer);
        }
        count--;
    }

    return count == 0;
}

/**
 * @note Prints all saved accounts with their passwords.
 */
bool print_all()
{
    char buffer[MAX_EXPECTED_LINE_LENGTH + 1];

    FILE *file = fopen(data_file, "r");
    if (file == NULL) {
        fprintf(stderr, "failed to open data_file\n");
        return false;
    }

    while (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) != NULL) {
        printf("\n%s", buffer);

        if (! print_site(buffer, file)) {
            fclose(file);
            return false;
        }
    }

    if (feof(file)) {
        fclose(file);
        return true;
    }

    fprintf(stderr, "failed to read a line\n");
    fclose(file);
    return false;
}

/**
 * @note You must read site name and account_count alone and then this reads through accounts and prints the password if
 *  the account is there.
 *
 * @param file File with data
 * @param account_count Number of accounts saved for this site
 * @param account_name What we are looking for
 * @param buffer at least 1000 characters long
 * @return true if no error occurs, false otherwise
 */
bool find_and_print_password(FILE *file, long account_count, char *account_name, char *buffer)
{
    for (long i = 0; i < account_count; i++) {
        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
            fprintf(stderr, "failed to read a line - data file was probably altered\n");
            return false;
        }

        if (strcmp(buffer, account_name) == 0) {
            if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
                fprintf(stderr, "failed to read a line - data file was probably altered\n");
                return false;
            }
            printf("is:\n%s", buffer);
            return true;
        }

        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
            fprintf(stderr, "failed to read a line - data file was probably altered\n");
            return false;
        }
    }
    fprintf(stderr, "was not found. Double check if you wrote the site and account name correctly.\n");
    return true;
}

/**
 * @param site_name On what site is this account.
 * @param account_name Name of the account we want password of.
 * @return true if no error occurs, false otherwise
 */
bool print_password(char *site_name, char *account_name)
{
    FILE *file = fopen(data_file, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open the file with data.\n");
        return false;
    }

    char buffer[MAX_EXPECTED_LINE_LENGTH + 1];

    while (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) != NULL) {
        if (strcmp(buffer, site_name) != 0) {
            if (! skip_to_another_site(file, NULL, buffer)) {
                fclose(file);
                return false;
            }
            continue;
        }

        size_t length = strlen(buffer) + 1;
        char *name = malloc(length * sizeof(char));
        if (name == NULL) {
            fclose(file);
            fprintf(stderr,"malloc failed\n");
            return false;
        }

        memcpy(name, buffer, length);

        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH + 1, file) == NULL) {
            free(name);
            fclose(file);
            fprintf(stderr, "failed to read a line - data file was probably altered\n");
            return false;
        }

        long count = strtol(buffer, NULL, 10);
        if (0 >= count || errno == ERANGE) {
            fprintf(stderr, "data file was probably altered\n");
            free(name);
            fclose(file);
            return false;
        }

        printf("The password for this account ");
        if (! find_and_print_password(file, count, account_name, buffer)) {
            free(name);
            fclose(file);
            return false;
        }

        free(name);
        fclose(file);
        return true;
    }
    fprintf(stderr, "The password was not found. Double check if you wrote the site and account name correctly.\n");
    return true;
}

/**
 * @note Asks you whether you want to print all saved passwords or one particular password and prints it.
 */
bool print_account_info()
{
    char buffer[4];
    if (yes_no_question("Would you like to print passwords for all saved accounts?\n", buffer, 4)) {
        return print_all();
    }

    char *site_name = malloc((LONGEST_NAME + 1) * sizeof(char));
    if (site_name == NULL) {
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    char *account_name = malloc((LONGEST_NAME + 1) * sizeof(char));

    if (account_name == NULL) {
        free(site_name);
        fprintf(stderr, "malloc failed\n");
        return false;
    }

    printf("Please write which account's password you want to show:\n");

    if (fgets(account_name, LONGEST_NAME + 1, stdin) == NULL) {
        free(site_name);
        free(account_name);
        fprintf(stderr, "failed to read input\n");
        return false;
    }

    printf("Please write to what site is this account:\n");

    if (fgets(site_name, LONGEST_NAME + 1, stdin) == NULL) {
        free(site_name);
        free(account_name);
        fprintf(stderr, "failed to read input\n");
        return false;
    }

    bool result = print_password(site_name, account_name);

    free(site_name);
    free(account_name);

    return result;
}
