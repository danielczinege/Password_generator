#ifndef PASSWORD_GENERATOR_DATA_SAVING_H
#define PASSWORD_GENERATOR_DATA_SAVING_H

#include <stdbool.h>

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

#endif //PASSWORD_GENERATOR_DATA_SAVING_H
