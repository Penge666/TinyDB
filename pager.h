#ifndef PAGER_H
#define PAGER_H
#include "const.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "fcntl.h"
#include "unistd.h"
#include <stdint.h>

// #define PAGE_SIZE 4096
// #define TABLE_MAX_PAGES 100

typedef struct {
  int file_descriptor;
  uint32_t file_length;
  uint32_t num_pages;
  void* pages[TABLE_MAX_PAGES];
} Pager;
void pager_flush(Pager* pager, uint32_t page_num) ; 
Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_num); 
#endif