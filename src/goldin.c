
/* goldin.c - Golden indexer main functions */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <libgen.h>
#include <errno.h>
#include <err.h>

#include "error.h"
#include "list.h"
#include "entry.h"
#include "index.h"
#include "index_desc.h"
#include "access.h"
#include "locus.h"


#include "goldin_options.h"
#include "goldin_utils.h"

#define PERF_PROFILE

#ifdef PERF_PROFILE
#include <time.h>
#endif

#include <fcntl.h>

/* Global variables */
// static char *prog;


void process_databank_files(int,int,char ** ,goldin_parms);
all_indix_nb process_databank_file(goldin_parms, char * );

void process_index_files(int,int,char **,goldin_parms);

/* Main function */
int main(int argc, char **argv) {
  FILE *f;
  int i;
  char *p, *dbase, *file;
  entry_t ent;
  long locnb, accnb, indnb;
  indix_t *cur, *locind, *accind;
  size_t len;
  goldin_parms s_parms;

  printf("coucou\n");

  /* Checks command line options & arguments */
  init_goldin_parms(&s_parms,argc, argv);
  /* Proceed all input files */

  if (!s_parms.idx_input_flag) {
    process_databank_files(optind,argc,argv,s_parms);
  } else {/* I am expecting index files as input */
    process_index_files(optind,argc,argv,s_parms);
  }
  return EXIT_SUCCESS;
}

void process_databank_files(int optind,int argc,char ** argv,goldin_parms s_parms) {
  int i;
  char* file;
  all_indix_nb tot_idx;
  for(i = optind + 1; i < argc; i++) {
     file = argv[i];
     tot_idx=process_databank_file(s_parms,file);
  }
  if (s_parms.csort_flag) { // sort index file.
    all_index_sort(s_parms,tot_idx);
  }
}



all_indix_nb process_databank_file(goldin_parms s_parms , char * file) {
  struct stat st;
  int nb;
  int ret;
  all_indix_nb tot_idx;
  /* Check for regular file */
  if (stat(file, &st) == -1) err(errno,file, NULL);
  if ((st.st_mode & S_IFMT) != S_IFREG) err(errno, file, "not a regular file");

  /* Add file to list */
  nb = list_append(s_parms.dbase, s_parms.dir, file,s_parms.new_index_dir);
  all_indix_t file_l_indix=create_index(file,nb,s_parms.loc,s_parms.acc);
  tot_idx.accnb=file_l_indix.accnb;
  tot_idx.locnb=file_l_indix.accnb;
  if (s_parms.wrn && (file_l_indix.locnb + file_l_indix.accnb) == 0) {
     warn("%s %s",file, "file contains no entries");
     return tot_idx;
  }
  if (!s_parms.csort_flag && !s_parms.co_flag) { // same behavior as in previous versions.
    all_index_mmerge(file_l_indix,s_parms);
    freeAllIndix(file_l_indix);
    return tot_idx;
  }
  // concatenate new index with previous ones.
  tot_idx=all_index_mconcat(file_l_indix,s_parms);
  freeAllIndix(file_l_indix);
  return tot_idx;
}

void process_index_file(goldin_parms s_parms ,char * rac_file,  dest_index_desc * d_descr) {
  source_index_desc s_descr;
  int nb;
  char * s_dbx_file;
  char * l_flats;


  s_descr=get_source_index_desc(s_parms.acc,s_parms.loc,s_parms.new_index_dir,rac_file);
  s_dbx_file=index_file(s_parms.new_index_dir,rac_file,LSTSUF);
  l_flats=list_get(s_dbx_file);

  // concatenate
  nb = list_append(s_parms.dbase, s_parms.dir,l_flats,s_parms.new_index_dir);
  free(l_flats);
  if (s_parms.acc) {
    if (fseeko(d_descr->d_facx, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.acx",s_parms.dbase);
    d_descr->accnb=index_file_concat(d_descr->d_facx,d_descr->max_filenb, s_descr.accnb, s_descr.d_facx,d_descr->accnb);
  }
  if (s_parms.loc) {
    if (fseeko(d_descr->d_ficx, 0, SEEK_END) == -1) err(errno,"error while getting at the end of file: %s.icx",s_parms.dbase);
    d_descr->locnb=index_file_concat(d_descr->d_ficx,d_descr->max_filenb, s_descr.locnb, s_descr.d_ficx,d_descr->locnb);
  }
  // close source index files
  close_source_index_desc(&s_descr);
  d_descr->max_filenb=nb;
}

void process_index_files(int optind,int argc,char ** argv,goldin_parms s_parms) {
  int i;
  char* rac_file;

  all_indix_nb tot_idx;
  dest_index_desc d_descr;

  d_descr=get_dest_index_desc(s_parms.acc,s_parms.loc,s_parms.new_index_dir,s_parms.dbase); // get description of destination index files.
  for(i = optind + 1; i < argc; i++) {
     rac_file = argv[i];
     process_index_file(s_parms ,rac_file,&d_descr);
  }
  // update dest index files with right number of indexes.
  if (s_parms.acc) {
    if (fseeko(d_descr.d_facx, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.acx",s_parms.dbase);
    if (fwrite(&d_descr.accnb, sizeof(d_descr.accnb), 1, d_descr.d_facx) != 1) err(errno,"error writing number of indexes");
  }
  if (s_parms.loc) {
    if (fseeko(d_descr.d_ficx, 0, SEEK_SET) == -1) err(errno,"error while getting at the beginning of file: %s.acx",s_parms.dbase);
    if (fwrite(&d_descr.locnb, sizeof(d_descr.accnb), 1, d_descr.d_ficx) != 1) err(errno,"error writing number of indexes");
  }

  // close dest index files
  close_dest_index_desc(&d_descr);

  tot_idx.accnb=d_descr.accnb;
  tot_idx.locnb=d_descr.locnb;
  if (s_parms.csort_flag) { // sort index file.
    all_index_sort(s_parms,tot_idx);
  }
}




