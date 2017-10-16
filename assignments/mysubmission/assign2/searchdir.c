/*
 * File: searchdir.c
 * Author: Tiantian Tang
 * ------------------------------------------------------------
 * This program will recursively search a directory provided
 * by user.
 * Usage: ./searchdir [one of: -i or -d or searchstr] [optional: directory name]
 * Depending on the argument, perform following tasks
 *  1. Name Search: find if directory contains any filename contains
 *  searchstr. If so, print out all full paths for such files lexicographically
 *  2. Inode Search:  find if directory contains any filename has  inode provided
 *  by user input. If so, print out all full paths for such files 
 *  3. Date Search: find if directory contains any filename has date  provided
 *  by user input. If so, print out all full paths for such  files 
 */

#include "cmap.h"
#include "cvector.h"
#include <dirent.h>
#include <error.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


#define NFILES_ESTIMATE 20
#define MAX_INODE_LEN   21  // digits in max unsigned long is 20, plus \0
#define DATE_MAX 6 // mm/dd plus \0
typedef void (*GatherFn)(const char *fullpath, struct stat ss, void *aux);

/*
 * Function: match_helper
 * ------------------------------------------------------------
 * Return true if input and pattern character matches
 */

static bool match_helper(const char *input, const char*pattern){
    if (pattern[0] == '\0') return true;
    if (input[0] == pattern[0]) return match_helper(input + 1, pattern + 1);
    return false;
}

/*
 * Function: match
 * ------------------------------------------------------------
 *  Return true if searchstr match any substr of filename
 */
static bool match(const char *searchstr, const char *filename){
    for (int i = 0; filename[i] != '\0'; i++){
        if (match_helper (filename + i, searchstr)) return true;
    } 
    return false;
}

/*
 * Function: cmp_ino
 * ------------------------------------------------------------
 * compare function for inodes based on numeric order of two
 *  arguments
 */

int cmp_ino(const void *addr1, const void *addr2){
    if (*(unsigned long *)addr1 <  *(unsigned long *)addr2) return -1; 
    if (*(unsigned long *)addr1 >  *(unsigned long *)addr2) return 1;
    return 0;
}
/*
 * Function: clean
 * ------------------------------------------------------------
 * clean up function for cleaning heap allocation that element
 * points to
 */
void clean(void *element){
    free(*(char**)element);
}
/*
 * Function: cleanDateMap
 * ------------------------------------------------------------
 * clean up function for cleanning heap allocatoin that 
 * CVector * element in the CMap point to
 */

void cleanDateMap(void *element){
    // each value store in Cmap is a pointer to Cvector store in heap
    // need to dispose each of them
    CVector *pathVec = *(CVector **) element;
    cvec_dispose(pathVec);
}

/*
 * Function: gather_vector
 * ------------------------------------------------------------
 * gather function to store pointer to fullpath string into CVector
 */
void gather_vector(const char *fullPathPtr, struct stat ss, void *aux) {
    char *mallocptr = strdup(fullPathPtr); 
    CVector *matches = (CVector *) aux; // Unpack the generic auxiliary data
    // fullpath must be on the heap!
    cvec_append(matches, &mallocptr); 
}
/*
 * Function: gather_map
 * ------------------------------------------------------------
 * gather function to put <inode string, &fullpathptr) into map
 */ 
void gather_map(const char *fullPathPtr, struct stat ss, void *aux) {
    char *mallocptr = strdup(fullPathPtr); 

    CMap *map = (CMap*) aux;// unpack generic auxiliary data
    unsigned long inode = ss.st_ino; 
    char inodestr[MAX_INODE_LEN];
    char *inodeptr;
    inodeptr = inodestr;
    sprintf(inodeptr, "%lu", inode);
    cmap_put(map, inodestr, &mallocptr);
}
/*
 * Function: gather_dateMap 
 * ------------------------------------------------------------
 * gather function to put <key, value> pair of 
 * <pointer to fullpath, pointer to cvector> in dateMap
 * the Cvector contains list of points to ull path of same
 * inode date.
 *
 */
void gather_dateMap(const char *fullPathPtr, struct stat ss, void *aux){


    char *mallocptr = strdup(fullPathPtr); 
    CMap *dateMap = (CMap *) aux; // unpack generic auxiliary data
    struct tm *timeobj = localtime(&(ss.st_mtime));
    char datebuf[DATE_MAX]; 
    strftime(datebuf, DATE_MAX, "%m/%d", timeobj);

    void *val;// generic pointer to receive
    val = cmap_get(dateMap, datebuf);
    CVector *cvecPaths;

    if (val != NULL){
        // when exisits value for key ==  MM/DD in the datemap
        cvecPaths = *(CVector **)val;
        // append fullpath pointer to the cvecPaths
        cvec_append(cvecPaths, &mallocptr);

    }else{
        // when no value for key == MM/DD, need to creacte a Cvec
        cvecPaths = cvec_create(sizeof(char *),10,clean);
        // store pointers to path in cvec, and put key, value pair in dateMap
        cvec_append(cvecPaths, &mallocptr);
        cmap_put(dateMap, datebuf, &cvecPaths);
    }

}
/*
 * Function: gather_files
 * ------------------------------------------------------------
 * visit directories of given dirname recursively, record visited
 * inodes. Also add information to data sturcture by invoding provided
 * gatherer function
 */
static void gather_files(CVector *matches, const char *searchstr, const char *dirname, CVector *visited, void *aux, GatherFn gatherer)
{
    DIR *dp = opendir(dirname); 
    struct dirent *entry;

    while (dp != NULL && (entry = readdir(dp)) != NULL) { /* iterate over entries in dir */
        if (entry->d_name[0] == '.') continue; /* skip hidden files */
        // form fullpath
        char fullpath[sizeof(char) * strlen(dirname) + strlen(entry->d_name) + 2];
        char *fullPathPtr;
        fullPathPtr = fullpath;
        strcpy(fullPathPtr, dirname);
        strcat(fullPathPtr, "/");
        strcat(fullPathPtr, entry->d_name);

        struct stat ss;
        int result = stat(fullPathPtr, &ss);
        if (result == 0 && S_ISDIR(ss.st_mode)) {  /* if subdirectory, recur */

            unsigned long inode = ss.st_ino;    /* inode number is unique id per entry in filesystem */
            if (cvec_search(visited, &inode, cmp_ino, 0, false) == -1){
                // when such inode is not found, add to visited CVctor
                // and keep searching in subdirectoryu
                cvec_append(visited, (unsigned long*)&inode);
                gather_files(matches, searchstr, fullPathPtr, visited, aux, gatherer);
            }
        } else {
            // if search str and file path match, gather information and store to
            // aux data structure specified by caller
            if (match(searchstr, entry->d_name)) {
                gatherer(fullPathPtr,ss, aux);
            }
        }
    }
    closedir(dp);
}


/*
 * Function: cmp_path 
 * ------------------------------------------------------------
 *  compare function for paths. First compare by path lenght
 *  then compare by lexicographical order
 */
int cmp_path(const void *addr1, const void *addr2){
    char *ptr1 = * (char**) addr1;
    char *ptr2 = * (char**) addr2;
    int size1 = strlen(ptr1);
    int size2 = strlen(ptr2);

    if (size1 < size2) return -1;
    if (size1 > size2) return 1;
    // when str size equal, compare lexicographicall
    for (int i = 0; *(ptr1 + i)!= '\0'; i++){
        int charCmpRes = strcmp(ptr1 + i, ptr2 + i);
        if ((charCmpRes!= 0)) return charCmpRes; 
    }
    return 0;

}
/*
 * Function: inodeSearch 
 * ------------------------------------------------------------
 * ask user for inode number and find if any file with such inode
 * exist in directory. If so, print out full path.
 * if user enter 'q', quit the program
 */
void inodeSearch(CMap *map){
    char input[MAX_INODE_LEN];
    strcpy(input, "");//initialize input first
    int c;
    c = 1;
    char *valPtr;
    void *ptrToVal;

    while (strcmp(input, "q") != 0 && c > 0){
        printf("Enter inode (or q to quit): ");
        c = scanf("%s", input);
        ptrToVal= cmap_get(map, input);
        if (ptrToVal != NULL){
            valPtr = *(char **)ptrToVal;
            printf("%s\n",valPtr);
        }
    }

}

/*
 * Function:  dateSearch 
 * ------------------------------------------------------------
 * ask user for date in MM/DD and find if any file with date
 * exist in directory. If so, print out full path.
 * if user enter 'q', quit the program
 */
void dateSearch(CMap *dateMap){
    char input[DATE_MAX];
    strcpy(input, "");//initialize input first
    int c;
    c = 1;
    void *ptrToVal;
    char *pathPtr;
    while (strcmp(input, "q") != 0 && c > 0){
        printf("Enter date MM/DD (or q to quit):  ");
        c = scanf("%s", input);
        ptrToVal= cmap_get(dateMap, input);
        if (ptrToVal != NULL){
            CVector *pathVec = * (CVector **) ptrToVal;
            for (void *cur = cvec_first(pathVec); cur != NULL; cur = cvec_next(pathVec, cur)){
                pathPtr = *(char **)cur;
                printf("%s\n",pathPtr);
            }

        }
    }
}
/*
 * Function: namesearch 
 * ------------------------------------------------------------
 * depends on option, apply corresponding data structure
 * and gather function to gather_files to retrieve information
 * of  given directory
 */
static void namesearch(const char *searchstr, const char *dirname, int option)
{
    // Cvector to store uniqu inode that has been visited
    CVector *visited = cvec_create(sizeof(unsigned long), 10, NULL);    
    // CVector to store point to matched path string for searchstr
    CVector *matches = cvec_create(sizeof(char*), NFILES_ESTIMATE, clean);
    // CMap to store store <inode, &fullpath> for inode search 
    CMap *map = cmap_create(sizeof(char*), 10,clean);
    // CMap to store <string, cvector *> pair about  MM/DD and pointer to Vector 
    // that contains full paths for date search
    CMap *dateMap = cmap_create(sizeof(CVector *), 10, cleanDateMap);

    if (option == 0){// option 0: searchstr
        gather_files(matches, searchstr, dirname, visited, matches, gather_vector);
        cvec_sort(matches, cmp_path); //sort element stored in matches by length, then lexicograph
        for (char **cur = cvec_first(matches); cur != NULL; cur = cvec_next(matches, cur)){
            printf("%s\n", *cur);
        }
    }else if (option == 1){//option 1: -i search
        gather_files(matches, searchstr, dirname, visited, map, gather_map);
        // ask user for input and print out result
        inodeSearch(map);
    }else if (option == 2){ // option2: -d search
        gather_files(matches, searchstr, dirname, visited, dateMap, gather_dateMap);
        // ask user for input and print out result
        dateSearch(dateMap);
    }
    // clean up heap memory before existing function
    cvec_dispose(matches);
    cvec_dispose(visited);
    cmap_dispose(map);
    cmap_dispose(dateMap);
}

/*
 * Function: main
 * ------------------------------------------------------------
 * main function for program
 */
int main(int argc, char *argv[])
{

    if (argc < 2) error(1,0, "Usage: searchdir [-d or -i or searchstr] [(optional) directory].");

    char *dirname = argc < 3 ? "." : argv[2];
    if (access(dirname, R_OK) == -1) error(1,0, "cannot access path \"%s\"", dirname);
    char emptyStr[1];

    if (0 == strcmp("-d", argv[1])) {
        // search by date
        emptyStr[0] = '\0';
        namesearch(emptyStr, dirname,2);
    } else if (0 == strcmp("-i", argv[1])) {
        // search by inode
        emptyStr[0] = '\0';
        namesearch(emptyStr, dirname,1);
    } else {
        // search by name
        namesearch(argv[1], dirname, 0);
    }
    return 0;
}

