#include "data_saving.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

//Because I allow passwords to be 999 characters long
#define MAX_EXPECTED_LINE_LENGTH 1000

const char *data_file = "file";
const char *aux_file = "aux";

/**
 * @note Reads accounts and saves them to accounts until account_name is found.
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
        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) == NULL) {
            fprintf(stderr, "failed to read a line - data file was probably altered\n");
            return false;
        }

        if (strcmp(buffer, account_name) == 0) {
            if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) == NULL) {
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

        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) == NULL) {
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
 * @param write file where we write data
 * @param buffer at least 1000 characters long
 * @return true if no error occurs, false otherwise
 */
bool skip_to_another_site(FILE *file, FILE * write, char *buffer)
{
    if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) == NULL) {
        fprintf(stderr, "failed to read a line - data file was probably altered\n");
        return false;
    }

    long count = strtol(buffer, NULL, 10);
    if (0 >= count || errno == ERANGE) {
        fprintf(stderr, "data file was probably altered\n");
        return false;
    }

    fprintf(write, "%ld\n", count);

    count *= 2;

    while (count > 0 && fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) != NULL) {
        fprintf(write, "%s", buffer);
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

    while (fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) != NULL) {
        if (strcmp(buffer, site_name) != 0) {
            fprintf(write, "%s", buffer);
            if (! skip_to_another_site(file, write, buffer)) {
                fclose(file);
                fclose(write);
                return false;
            }
            continue;
        }
        fprintf(write, "%s", buffer);

        if (fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) == NULL) {
            fclose(file);
            fclose(write);
            fprintf(stderr, "failed to read a line - data file was probably altered\n");
            return false;
        }

        long count = strtol(buffer, NULL, 10);
        if (0 >= count || errno == ERANGE) {
            fprintf(stderr, "data file was probably altered\n");
            fclose(write);
            fclose(file);
            return false;
        }

        struct account_info **accounts = malloc(count * sizeof(*accounts));
        if (accounts == NULL) {
            fprintf(stderr, "malloc failed\n");
            fclose(write);
            fclose(file);
            return false;
        }

        int loaded_count = 0;
        bool found_account = false;

        if (! load_until_account(account->account_name, count, accounts, buffer, file, &found_account, &loaded_count)) {
            free_accounts(accounts, loaded_count);
            fclose(file);
            fclose(write);
            return false;
        }

        if (found_account) {
            fprintf(write, "%ld\n", count);
        } else {
            fprintf(write, "%ld\n", count + 1);
        }

        write_loaded_accounts(accounts, loaded_count, write);
        free_accounts(accounts, loaded_count);

        if (account->password != NULL) {
            fprintf(write, "%s", account->account_name);
            fprintf(write, "%s", account->password);
        }

        while (fgets(buffer, MAX_EXPECTED_LINE_LENGTH, file) != NULL) {
            fprintf(write, "%s", buffer);
        }
        if (feof(file)) {
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
        fclose(write);
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
