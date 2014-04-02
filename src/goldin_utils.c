/*
 * Here : aim is to make goldin.c simplier.
 * I will gather here "high level" functions that are used only by goldin.
 * These functions may use directly those in index.c, list.c,locus.c,access.c.
 */
#include <errno.h>
#include <err.h>
#include "index.h"
#include "goldin_options.h"

void all_index_sort(goldin_parms s_parms,all_indix_nb tot_idx) {
  char *file;
  if (s_parms.loc) {
    file = index_file(s_parms.new_index_dir, s_parms.dbase, LOCSUF);
    index_sort(file,tot_idx.locnb);
  }
  if (s_parms.acc) {
    file = index_file(s_parms.new_index_dir, s_parms.dbase, ACCSUF);
    index_sort(file,tot_idx.accnb);
  }
}


all_indix_nb mem_index_concat(all_indix_t file_l_indix,goldin_parms s_parms) {
  all_indix_nb tot_idx;

  if (s_parms.loc) {
    tot_idx.locnb=locus_concat(s_parms.dbase, file_l_indix.locnb, file_l_indix.l_locind,s_parms.new_index_dir);
    if (tot_idx.locnb==IDX_ERR) err(errno,"entry names indexes concatenation failed : %s",s_parms.dbase);
  }
  if (s_parms.acc) {
    tot_idx.accnb=access_concat(s_parms.dbase, file_l_indix.accnb, file_l_indix.l_accind,s_parms.new_index_dir);
    if (tot_idx.accnb==IDX_ERR) err(errno,"accession numbers indexes concatenation failed : %s",s_parms.dbase);
  }
  return tot_idx;
}


/*
 * Merge new indexes with existing index file.
 * Compute cpu time and wall time for this operation if program was built with PERF_PROFILE
 */
void mem_index_merge(all_indix_t file_l_indix,goldin_parms s_parms) {

  /* Merge indexes */
  if (s_parms.loc) {
    if (locus_merge(s_parms.dbase, file_l_indix.locnb, file_l_indix.l_locind,s_parms.new_index_dir)) err(errno,"entry names indexes failed : %s",s_parms.dbase);
  }
  if (s_parms.acc) {
    if (access_merge(s_parms.dbase, file_l_indix.accnb, file_l_indix.l_accind,s_parms.new_index_dir)) err(errno,"accession numbers indexes failed : %s",s_parms.dbase);
  }

}