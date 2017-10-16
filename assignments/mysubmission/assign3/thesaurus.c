/* File: thesaurus.c
 * -----------------
 * A program that uses CVector/CMap to build a thesaurus of synonyms. The CMap
 * associates words with CVectors of other words. The thesaurus file is huge,
 * so this serves as a scalability test.
 * jzelenski, based on earlier program by Jerry Cain
 */

#include <stdio.h>
#include "cmap.h"
#include "cvector.h"
#include <stdlib.h>
#include <string.h>
#include <error.h>

#define NUM_SYNONYMS 16
#define NUM_HEADWORDS 35000

static void cleanup_cvec(void *p)
{
    cvec_dispose(*(CVector **)p);
}

static void cleanup_str(void *p)
{
    free(*(char **)p);
}

/**
 * Reads a single line from FILE * using fgets into the client's
 * buffer. Removes the newline and returns true if line was non-empty
 * false othewise.
 */
static bool read_line(FILE *fp, char *buf, int sz)
{
    if (!fgets(buf, sz, fp)) return false;
    int last = strlen(buf) - 1;
    if (buf[last] == '\n') buf[last] = '\0'; // strip newline
    return (*buf != '\0');
}

/**
 * Tokenizes thesaurus data file and builds map of word -> synonyms.
 * Each line of data file is expected to be of the form:
 *
 *     cold,arctic,blustery,freezing,frigid,icy,nippy,polar
 *
 * The first word (or phrase) is primary, and rest of line are synonyms of first.
 * The ',' delimits words, and the '\n' marks the end of the entry.
 */
static CMap *read_thesaurus(FILE *fp)
{
    CMap *thesaurus = cmap_create(sizeof(CVector *), NUM_HEADWORDS, cleanup_cvec);
    printf("Loading thesaurus..");
    fflush(stdout);

    char line[10000], buffer[128];
    while (read_line(fp, line, sizeof(line))) { // read one line
        if (line[0] == '#') {               // echo file comment
            printf(" (%s)", line+1);
            continue;
        }
        char *cur = line;
        sscanf(line, "%127[^,]", buffer);   // first word of line is headword
        cur += strlen(buffer);
        CVector *synonyms = cvec_create(sizeof(char *), NUM_SYNONYMS, cleanup_str);
        cmap_put(thesaurus, buffer, &synonyms);
        while (sscanf(cur, ",%127[^,]", buffer) == 1) { // all subsequent words are synonyms
            char *synonym = strdup(buffer);
            cvec_append(synonyms, &synonym);
            cur += strlen(buffer) + 1;
        }
        if (cmap_count(thesaurus) % 1000 == 0) {
            printf(".");
            fflush(stdout);
      }
   }
   printf(".done.\n");
   fclose(fp);
   return thesaurus;
}

/**
 * Simple question loop that prompts the user for a entry, and
 * then looks it up in the thesaurus.  If present, it prints the
 * list of synonyms found.
 */
static void query(CMap *thesaurus)
{
    while (true) {
        char response[1024];
        printf("\nEnter word (RETURN to exit): ");
        if (!read_line(stdin, response, sizeof(response))) break;
        CVector **found = (CVector **)cmap_get(thesaurus, response);
        if (found != NULL) {
            char **cur = cvec_first(*found);
            printf("%s: {%s", response, cur ? *cur : "");
            while (cur && (cur = cvec_next(*found, cur)))
                printf(", %s", *cur);
            printf("}\n");
        } else {
            printf("Nothing found for \"%s\". Try again.\n", response);
        }
    }
}

int main(int argc, char *argv[])
{
    const char *filename = (argc == 1) ? "/afs/ir/class/cs107/samples/assign3/thesaurus.txt" : argv[1];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) error(1, 0,"Could not open thesaurus file named \"%s\"", filename);
    CMap *thesaurus = read_thesaurus(fp);
    query(thesaurus);
    cmap_dispose(thesaurus);
    return 0;
}

