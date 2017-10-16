/*
 * File: mygrep.c
 * Student:Tiantian Tang 
 * -----------------------
 *
 * The 'mygrep' program implements a simplified version of the 'grep' command.
 * The starter code below is a program skeleton that is a good beginning, but
 * is incomplete. Your edits to this program will involve both modifying the 
 * existing code and adding new code. Your first task is to read and thoroughly 
 * understand what is given. Take the time to study what it is doing and how 
 * it works, and definitely look into any techniques or libraries used that are 
 * unfamiliar to you (e.g. pointer subtraction or printf string precision).
 * Once you understand exactly what the code does (and doesn't do), add
 * appropriate commenting to document your understanding. You are now in the 
 * perfect position from which to proceed on modifying/extending the code for the 
 * assignment requirements.
 */

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LEN 1024

/* 
 * Emphasis: terminal codes are used to control the font characteristics of
 * of the output characters. We will use inverse video to show emphasis.
 * Terminal codes are a bit goopy and nothing worth investigating deeply, 
 * all you need to do is output this OPEN/CLOSE around the characters you want
 * emphasized. See given code for print_with_emphasis() as an example.
 */
#define OPEN_EMPHASIS "\e[7m"
#define CLOSE_EMPHASIS "\e[0m"

/* Function: regex_match
 * ---------------------------------------------------------------
 *  Given pointers to character starting at certain position input and patterns, 
 *  finds out if str starting at input and pattern match
 *
 *  base case:
 *          when pattern ptr arrives at terminating character  set end ptrs and return true
 *          or when input arrives at terminatin character and pattern still contains two character with second
 *          character *
 *  case 1: both pattern[0] and input[0] are characters. recursively check their next
 *          characters if equal. return false if not equal
 *  case 2: patern is now a '.' and pattern[1] != '*', which matches any character. recursively check their next
 *          characters if equal
 *  case 3: pattern[1] is a '*':
 *          if current characters don't match, try to match input[0] with pattern[2]
 *          if current characters match, go as far as possible in input string, check if input + 1, pattern + 2
 *          is true. if not, backtrack, untill input out of boundary or any solution during backtrackng is true.
 *          then check for rest of input and pattern.
 */
bool regex_match(const char *input, const char *pattern, const char **endp)
{
    // base case (end of pattern string--whole pattern matched)
    if (pattern[0] == '\0' || (strlen(pattern) == 2 && pattern[1] == '*' && input[0] == '\0')) {
        *endp = input;
        return true;
    }

    if (pattern[1] != '\0' && pattern[1] == '*'){
        const char*  start; // record starting position of current input
        start = input;
        // if current chararacters don't match
        if (pattern[0] != input[0] && pattern[0] != '.'){
            // try to match input[0] with pattern[2] which is the char after '*'
            return regex_match(input, pattern + 2, endp);

        }else{// if current char match
            //go as far as you can
            while ((input[1] == input[0] && input[0] == pattern[0]) || (pattern[0] == '.' && input[1] != '\0')) input++;
            // find solution during backtracking
            while (regex_match(input + 1, pattern + 2, endp) != true && input >= start ) input--;
            return regex_match(input + 1, pattern + 2, endp);
        }
    }        

    if (input[0] == pattern[0] || (pattern[0] == '.' && input[0] != '\0')) {
        return regex_match(input + 1, pattern + 1, endp);
    }


    // match failed
    return false;
}
/* Function: search 
 * ---------------------------------------------------------------
 *  Given pointers to input line and pattern
 *  find if there is match starting at any character in input.
 *  If there is, return pointer of starting char of first match
 *  on each line. Otherwise, return an NULL pointer.
 */
const char *search(const char *input, const char *pattern, const char **endp)
{
    for (int i = 0; input[i] != '\0'; i++) {
        if (pattern[0] == '^') {
            if (regex_match(input + i, pattern + 1, endp)) {
                return input + i;
            }
            // when ^ regex_match fails, use break to stop finding match on current line
            break;
        }else{
            if (regex_match(input + i, pattern, endp)) {
                return input + i;
            }
        }
    }
    *endp = NULL;
    return NULL;
}
/* Function: print_with_emphasis
 * ---------------------------------------------------------------
 *  print current line that cur points to with emphasis for matches
 */

void print_with_emphasis(const char *cur, const char *start, const char *end)
{
    int nbefore = start - cur;
    int nmatched = end - start;
    printf("%.*s%s%.*s%s", nbefore, cur, OPEN_EMPHASIS,
            nmatched, start, CLOSE_EMPHASIS);
}
/* Function: print_match
 * ---------------------------------------------------------------
 *  Read in one line of the file, find matches and print it 
 */

void print_match(const char *line, const char *pattern, const char *filename)
{
    const char *end = NULL;
    // start points to the starting char of found match
    // end points to the ending char of found match
    const char *start = search(line, pattern, &end);

    if (start != NULL) {
        if (filename != NULL) printf("%s: ", filename);
        print_with_emphasis(line, start, end);
        printf("%s\n", end);
    }
}
/* Function: grep_file
 * ---------------------------------------------------------------
 *  read in each line of the file and print matches if found
 * */
void grep_file(FILE *fp, const char *pattern, const char *filename)
{
    char line[MAX_LINE_LEN];

    while (fgets(line, sizeof(line), fp) != NULL) {
        // truncate trailing newline if present
        // (most lines will have one, last line might not)
        if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        print_match(line, pattern, filename);
    }
}
/*
 * Function: main
 * -----------------------------------------------------------------
 * mygrep program will accept pattern from user and find its matches
 * in filenames in argv[2] and after  or stdin if argc == 2
 *
 * */
int main(int argc, char *argv[])
{
    if (argc < 2) error(1, 0, "Usage: mygrep PATTERN [FILE]...");
    const char *pattern = argv[1];

    if (argc == 2) {
        grep_file(stdin, pattern, NULL);
    } else {
        // for each file in the argument, find match and print it
        for (int i = 2; i < argc; i++) {
            FILE *fp = fopen(argv[i], "r");
            if (fp == NULL) error(1, 0, "%s: no such file", argv[i]);
            grep_file(fp, pattern, argc > 3 ? argv[i] : NULL);
            fclose(fp);
        }
    }
    return 0;
}

