#ifndef PASSWORD_GENERATOR_DATA_SAVING_H
#define PASSWORD_GENERATOR_DATA_SAVING_H

#include <stdbool.h>

#define LONGEST_NAME 128

struct account_info {
    char *password;
    int password_length;

    char *account_name;
    int account_name_length;
};

bool save_or_delete_password(char *site_name, struct account_info *account);
bool get_and_save_password(void);
bool get_and_remove_password(void);
bool print_account_info();

#endif //PASSWORD_GENERATOR_DATA_SAVING_H
