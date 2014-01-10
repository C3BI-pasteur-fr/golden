/* access.c - Accession numbers functions */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "access.h"
#include "error.h"
#include "index.h"



/* Search in accession indexes */
/* VL: keep db_name parameter for recursive calls when searching in virtual database indexes.
 * lst_notFound points to a memory area allocated by the caller to store adresses of result_t structures for which no match was found.
 * It is used and "free" by the caller.
 * For recursive calls to access_search and calls to index_search, use memory areas allocated by access_search.
 */
void access_search(result_t ** lst_current_db, int lst_size, char * db_name, int * nb_AC_not_found) {
  FILE *f;
  char *p, *file, buf[1024];
  result_t ** start_l=lst_current_db; // for printing debug info

  // cur_base may be virtual.
  /* Virtual database indexes */
  file = index_file(NULL, db_name, VIRSUF);
  if (access(file, F_OK) != -1) {
    if ((f = fopen(file, "r")) == NULL) {
      error_fatal("memory", NULL); }

    while (fgets(buf, 1023, f) != NULL) {
      if ((p = strrchr(buf, '\n')) != NULL) { *p = '\0'; }
      access_search(lst_current_db, lst_size, buf, nb_AC_not_found);
#ifdef DEBUG
      // dump list of accession numbers that were not found yet.
      print_wrk_struct(lst_size,start_l,1);
      /*
      int i;
      result_t * cur_res;
      printf("Still looking for : ");
      for (i=0; i<lst_size; i++) {
    	  cur_res=*start_l;
        if (cur_res->filenb==NOT_FOUND) {
          printf("%s ",cur_res->name);
        }
        start_l++;
      }
      printf("\n"); */
#endif
      if (*nb_AC_not_found!=0) {
#ifdef DEBUG
    	  printf("access_search : all results were not found ; continue \n");
#endif
    	  continue;
      }
      break;
    }
    // if (fgetCalls>1 && in_lst_notFound.arrSize==0) { free(lst_current_db);} // if fgetCalls=1 or 0,lst_current_db is lstGoldenQuery's lst_work; and it is free in main.
                                                                            // if in_lst_notFound.arrSize!=0 : memory may need to be used after when looking for locus. => free it only if all AC were found.
    if (fclose(f) == EOF) {
      error_fatal("memory", NULL); }
    free(file);
    return;  }

  /* Real database indexes */
  file = index_file(NULL, db_name, ACCSUF);
#ifdef DEBUG
  //printf("Searching in file : %s \n",file);
#endif
  /*
  in_lst_notFound.addrArray=malloc(sizeof(result_t *));
  in_lst_notFound.arrSize=0;*/
  int nb_AC_found=index_search(file, db_name, lst_current_db,lst_size, nb_AC_not_found);
#ifdef DEBUG
  printf("access_search : returned from index_search: nb_not_found_for_db=%d, nb_res_found=%d \n",*nb_AC_not_found,nb_AC_found);
#endif
  // if everything was found, free memory.
  /*if (in_lst_notFound.arrSize==0) {
	  free(in_lst_notFound.addrArray); // pb here.
	  initArrayOfResAddr(&in_lst_notFound);
  }*/
  free(file);
  return;	}


/* Merge accession indexes */
int access_merge(char *dbase, long nb, indix_t *ind) {
  int i;
  char *file;

  file = index_file(".", dbase, ACCSUF);
  i = index_merge(file, nb, ind);
  free(file);

  return i; }
