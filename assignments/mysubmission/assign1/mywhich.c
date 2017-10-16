/*
 * File: mywhich.c
 * Student: Tiantian Tang
 * -----------------------
 *
 * The 'mywhich' program implements a simplified version of the 'which' command.
 * It is used to search for commands by name. It accepts one or more aguments
 * (each the name of a command) and searchs for matching executable in each of the 
 * directories in the PATH environment variable. It prints the full path of the
 * first matching exectuable found. The optional -p flag allows user to specify
 * the search path which is used instead of reading PATH from the environment.
 */

#include <stdio.h>
#include <string.h>

/* Function: main
 * --------------
 * Ordinarily main() has just two arguments (argc/argv); this alternate
 * prototype includes an optional third argument which is an array of
 * environment variables (like argc/argv, the values passed to this third
 * arg are set automatically by the operating system when the program is 
 * launched). Programs can use either form for main. The 2-argument 
 * version is more common, but mywhich will need this extra 3rd argument. 
 * (In fact, many programmers don't even know about this optional 3rd
 * argument, so now you're already elite! :-) )
 *
 * Each array entry in this third argument is a string (pointer char*), 
 * where the string pointed to is of the form "NAME=value", for different
 * environment variable NAMEs and configuration values. The array's last 
 * entry is a NULL pointer, which acts as a sentinel to identify the
 * end of the array (which is why we can get away with not including an 
 * accompanying array size argument, as we would normally do when passing 
 * arrays in C). 
 *
 * COURSE NOTE: This envp variable is introduced in lab1, so refer there
 * for more details!
 */

static void cutAndFind(char *dirs, char *filenames[], int size);
int main(int argc, char *argv[], char *envp[])
{
    // if no arguments was given, mywhich outputs nothing
    if (argc == 1){
        return 0;
    }else if (strcmp(argv[1], "-p") == 0){
        // handle -p case: find file in given directories
        cutAndFind(argv[2], argv + 3, argc - 3);
    }else{
        // iterate over enviroment vars comapre the PATH
        for (int i = 0; envp[i] != NULL; i++){
            if (strncmp(envp[i], "PATH=", 5) == 0){
                cutAndFind(envp[i] + 5, argv + 1, argc - 1);
                break;
            }
        }
    }

    return 0;

}
/* Function: cutAndFind
 * -----------------------------
 * print full path if command name has been found in environment paths seperated
 * by ":"
 *
 * returns: void
*/
static void cutAndFind(char *dirs, char *filenames[], int size){
    if (size > 0) {
        FILE *fp;
        char *token;

        for (int i = 0; i < size; i++){
            // local copy of dirs
            char dirLocal[strlen(dirs)];
            strcpy(dirLocal, dirs);
            // seperate directory by :
            token = strtok(dirLocal, ":");
            while (token != NULL){
                // form full path by copying and concatination
                char fullpath[strlen(token) + 2 + strlen(filenames[i])];
                strcpy(fullpath, token);
                strcat(fullpath, "/");
                strcat(fullpath, filenames[i]);
                fp = fopen(fullpath, "r");
                if (fp != NULL){
                    printf("%s\n", fullpath);
                    fclose(fp);
                    break;
                }
                token = strtok(NULL, ":");
            }
        }
    }



}

