/*
 * database.c 
 *
 * by Matthew Volk, John Clow, and CS107 staff
 *
 * The Heart of Darkness or at least the core of Wells' buggy code.
 * You are welcome to modify anything in this file, but note that
 * Wells Buggy has hired you to find the existing bugs in their
 * code, not to add new ones.
 *
 * Copyright 2016, 2017 Wells Buggy Inc.
 *
 */

#include "database.h"

#include <assert.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmap.h"

#define MAX_USER_LEN 32
#define MAX_PASS_LEN 32
#define MAX_FILE_LINE_LEN (MAX_USER_LEN + MAX_PASS_LEN + 100)
#define SENTINEL_PASS "***"

static WBDatabase *global_db = NULL;

struct WBDatabaseImplementation {
    void *login_user;
    char *naive_db;
    CMap *balances;
};

// STATIC HELPERS (look at assignment page for info on the static keyword)
static char *read_line(FILE *infile)
{
    char line[MAX_USER_LEN + MAX_PASS_LEN + 100];
    char *ptr = fgets(line, MAX_FILE_LINE_LEN, infile); 
    if (ptr == NULL) return NULL;
    return strdup(line);
}

static void clear_login() {
    free(global_db->login_user);
    global_db->login_user = NULL;
}

static void set_login(const char *user) {
    global_db->login_user = strdup(user);
}

static void set_balance(const unsigned int balance) {
    cmap_put(global_db->balances, global_db->login_user, &balance);
}

static char *build_user_pass(const char *user, const char *pass) {
    char *password_buf = malloc(strlen(user) + strlen(pass) + 3);
    sprintf(password_buf, "%s\t%s\n", user, pass);
    return password_buf;
}

static void debug_print_database(const char* naive_db) {
    printf("\n\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("                   DEBUG MODE ACTIVATED!                  \n");
    printf("\nDatabase contents:\n"); 
    printf("%s\n", naive_db);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

// DATABASE FUNCTIONS

void db_init(const char *init_file) {
	if (global_db) {
		printf("WBDatabase already initialized! Can't reinitialize database.\n");
		return;
	}

    printf("Generating test database from file %s... ", init_file);

    global_db = malloc(sizeof(WBDatabase));
    assert(global_db);
    char* login_db = strdup("");
    assert(login_db);
    global_db->balances = cmap_create(sizeof(unsigned int), 10, NULL);
    assert(global_db->balances);

    FILE *infile;

    if (!(infile = fopen(init_file, "r")))
        error(1, 0, "'%s': No such file", init_file);

    char *line, *user, *pass;
    int balance = 0;

    while ((line = read_line(infile))) {
        if (sscanf(line, "%ms\t%ms\t%d\n", &user, &pass, &balance) == 3) {
            char *buf = build_user_pass(user, pass);
            login_db = realloc(login_db, strlen(login_db) + strlen(buf) + 1);
            strcat(login_db, buf);
            cmap_put(global_db->balances, user, &balance);
            free(buf);
            free(user);
            free(pass);
			free(line);
        } else {
            error(1, 0, "Invalid database init file. Shutting down.");
        }
    }

    global_db->naive_db = login_db;
    printf("Done!\n\n");
}

void db_shutdown() {
    printf("Database shutting down... ");
    free(global_db->naive_db);
    cmap_dispose(global_db->balances);
    free(global_db);
    printf("Done!\n");
}

bool db_login(const char *user, const char *pass) {
    char user_buf[MAX_USER_LEN];
    int debug = 0;
    int authorized = 0;
    char pass_buf[MAX_PASS_LEN];

    strcpy(user_buf, user);
    strcpy(pass_buf, pass);

    if (debug) debug_print_database(global_db->naive_db);

    if (strcmp(pass_buf, SENTINEL_PASS) == 0) {
        printf("Cannot log in to an administrator account.\n");
        clear_login();
        return authorized;
    }

    char *buf = build_user_pass(user_buf, pass_buf);
    char *found = strstr(global_db->naive_db, buf);
    free(buf);

    if (!found) {
        printf("Invalid username / password combination.\n");
        clear_login();
        return authorized;
    } 
    set_login(user_buf);
    printf("Logged in as %s.\n", user_buf);
    authorized = 1;
    return authorized;
}

void db_logout() {
    clear_login();
    printf("Successfully logged out.\n");
}

char *db_get_user() {
	if (!global_db->login_user) return NULL;
    return strdup(global_db->login_user);
}

const unsigned int db_check_balance() {
    char *user = db_get_user();
    if (user == NULL) {
        printf("Please log in before trying to check balance.\n");
        return 0;
    }
    unsigned int *balance = cmap_get(global_db->balances, global_db->login_user);
    if (balance == NULL) {
        printf("User %s balance does not exist in database.\n", user);
		free(user);
        return 0;
    }
	free(user);
    return *balance;
}

const bool db_withdraw(const int amount) {
    char *user = db_get_user();
    if (user == NULL) {
        printf("Please log in before trying to withdraw funds.\n");
        return false;
    }
    unsigned int balance = *(unsigned int *) cmap_get(global_db->balances, user);
	free(user);
    if (amount > (int) balance) {
        printf("Overdraft protection activated! Attempted to withdraw $%d, but only have $%d in account.\n",
               amount, balance);
        return false;
    } 
    unsigned int new_balance = balance - amount;
    printf("Successfully withdrew $%d. New account balance: $%d.\n", amount, new_balance);
    set_balance(new_balance);
    return true;
}

bool db_change_password(const char *old_pass, const char *new_pass) {
    char *user = db_get_user();
    if (user == NULL) {
        printf("Please log in before trying to change passwords.\n");
        return false;
    }
    char *buf = build_user_pass(user, old_pass);
    char *found = strstr(global_db->naive_db, buf);
    free(buf);
    if (!found) {
        printf("Invalid username / password combination.\n");
        clear_login();
		free(user);
        return false;
    } 
    char *new_buf = build_user_pass(user, new_pass);
    strcpy(found, new_buf);
    printf("Successfully changed password.\n");

	free(new_buf);
	free(user);
    return true;
}

void db_destroy() {
    char *user = db_get_user();
    if (!user || strcmp(user, "ceo") != 0) {
        printf("Only the CEO can call this secure function.\nTerminating your connection, evil hacker!\n");
        exit(0);
    }
    printf("\n\n");
    printf("          __.-^^---....,,-_ \n");
    printf("      _--/                 --_\n");  
    printf("     <                       >)\n");
    printf("     |                        |\n"); 
    printf("     \\._                   _./\n");  
    printf("       ```--. . , ; .--'''\n");       
    printf("           .-=||  | |=-.\n");   
    printf("          `-=#$!&#!$#=-'\n");   
    printf("              |  ;  :|\n");     
    printf("_________.,-#!&$@!#&#~,._i________\n");
    printf("\n      ~~~  !!!KABOOM!!!  ~~~\n\n");
    printf("Congratulations! You passed level 3 and killed Wells Buggy.\n");
    exit(0);
}


