/*
 * File: simple.c
 * --------------
 * Little nonsense program that tests some simple dynamic allocation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "allocator.h"


typedef struct _cell {
   char *string;
   struct _cell *next;
} cell;
    
	
// Add a new cell to front of list, data for new cell is
// string s. head is passed by ref to change to point to new cell
static void push(cell **head, char *s)
{
   cell *c = (cell *)mymalloc(sizeof(cell));
   c->next = *head;
   c->string = mymalloc(strlen(s)+1);
   strcpy(c->string, s);
   *head = c;
}

// Print entire linked list
static void print_list(cell *head)
{
   printf("---------------------------------------------------------\n");
   for (cell *cur = head; cur != NULL; cur = cur->next)
      printf("%p = [\"%s\" %p]\n", (void *)cur, cur->string, (void *)cur->next);
   printf("---------------------------------------------------------\n");
}

// Free all linked list cells
static void free_list(cell *head)
{
   while (head != NULL) {
      cell *next = head->next;
      myfree(head->string);
      myfree(head);
      head = next;
   }
}

// Joins each pair of neighboring list cells into one combined
// cell whose data field is the concatenation of the two data
// fields. The unused components are freed.
static void concat_neighbors(cell *head)
{
   for (cell *cur = head; cur && cur->next != NULL; cur=cur->next) {
      cell *next = cur->next;
      cur->next = next->next;
      cur->string = myrealloc(cur->string, strlen(cur->string)+strlen(next->string)+2);
      int len = strlen(cur->string);
      cur->string[len] = '-';
      strcpy(cur->string+len+1, next->string);
      myfree(next->string);
      myfree(next);
   }
}

// Does some silly linked list creation and manipulation
// in order to exercise the heap allocator routines.
int main(int argc, char *argv[])
{
   cell *list = NULL;
   myinit();
   for (int i = 0; i < 200; i++) {
      char num[10];
      sprintf(num, "%d", i);
      push(&list, num);
   }
   print_list(list);
   while (list && list->next != NULL)
      concat_neighbors(list);
   print_list(list);
   free_list(list);
   return 0;
}

