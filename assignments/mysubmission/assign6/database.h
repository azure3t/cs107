/* 
 * database.h
 * by Matthew Volk, John Clow, and CS107 staff
 *
 * Internal implementations of the database, not including functions
 * a normal user doesn't have access to. Your job is to look at the
 * code for these functions and break into the bank! They don't even
 * have comments, because they never took CS107.
 *
 * Copyright 2016, 2017 Wells Buggy Inc.
 *
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <stdbool.h>


typedef struct WBDatabaseImplementation WBDatabase;


void db_init(const char *init_file);

void db_shutdown();

bool db_login(const char *user, const char *pass);

void db_logout();

const bool db_withdraw(const int amount);

bool db_change_password(const char *old_pass, const char *new_pass);

char *db_get_user();

const unsigned int db_check_balance();

void db_destroy();

#endif

