#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char expr[512];
  uint32_t value;

} WP;

void print_wp() ;
WP* new_wp(char *expr, int val) ;
void free_wp(int NO) ;
bool check_wp();

#endif
