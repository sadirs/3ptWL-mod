//==========================================================================
//        1          2          3          4        ^ 5          6          7

/*
============================================================================
NAME: common.c                               [wlcf]
Written by: S. Aviles et al.
Starting date: February 2026
Purpose: Common utility and protection routines
Language: C
============================================================================
*/

#include "common.h"


/*
Protected sprintf routine:

This routine formats a string safely into the destination buffer
using vsnprintf.

*/
void class_protect_sprintf(char* dest, char* tpl,...) {
  va_list args;
  va_start(args,tpl);
  vsnprintf(dest, 2048,tpl,args);
  va_end(args);
}

void class_protect_fprintf(FILE* stream, char* tpl,...) {
  va_list args;
  char dest[6000];
  va_start(args,tpl);
  vsnprintf(dest, 2048,tpl,args);
  va_end(args);
  fprintf(stream,"%s",dest);
}
/*
Protected memcpy routine:

This routine copies a memory block from one location to another.

*/
void* class_protect_memcpy(void* dest, void* from, size_t sz) {
  return memcpy(dest, from,sz);
}
/*
Title-counting routine:

This routine counts the number of tab-separated titles in a string.

*/
int get_number_of_titles(char * titlestring){
  int i;
  int number_of_titles=0;

  for (i=0; i<strlen(titlestring); i++){
    if (titlestring[i] == '\t')
      number_of_titles++;
  }
  return number_of_titles;
}

/**
 * Finds wether or not a file exists.
 *
 * @param fname  Input: File name
 * @return boolean
 */
/*
File-existence checking routine:

This routine checks whether a file exists and can be opened for reading.

*/
int file_exists(const char *fname){
  FILE *file = fopen(fname, "r");
  if (file != NULL){
    fclose(file);
    return TRUE;
  }

  return FALSE;

}

/**
 * Finds whether two doubles are equal or which one is bigger
 *
 * @param a Input: first number
 * @param b Input: second number
 * @return -1, 1 or 0
 */
/*
Double-comparison routine:

This routine compares two double values and is intended for use
with sorting functions such as qsort.

*/
int compare_doubles(const void *a,
                    const void *b){
  double *x = (double *) a;
  double *y = (double *) b;
  if (*x < *y)
    return -1;
  else if
    (*x > *y) return 1;
  return 0;
}

/**
 * This function detects if a string begins with a character,
 * ignoring whitespaces during its search
 *
 * returns the result, NOT the SUCCESS or _FAILURE_ codes.
 * (This is done such that it can be used inside of an if statement)
 *
 * @param thestring  Input: string to test
 * @param beginchar  Input: the character by which the string begins or not
 * @return boolean
 */
/*
String-begin test routine:

This routine detects whether a string begins with a given character,
ignoring leading whitespaces.

*/
int string_begins_with(char* thestring, char beginchar){

  /** Define temporary variables */
  int int_temp=0;
  int strlength = strlen((thestring));
  int result = FALSE;

  /** Check through the beginning of the string to see if the beginchar is met */
  for(int_temp=0;int_temp<strlength;++int_temp){
    /* Skip over whitespaces (very important) */
    if(thestring[int_temp]==' ' || thestring[int_temp]=='\t'){continue;}
    /* If the beginchar is met, everything is good */
    else if(thestring[int_temp]==beginchar){result=TRUE;}
    /* If something else is met, cancel */
    else{break;}
  }

  return result;
}
