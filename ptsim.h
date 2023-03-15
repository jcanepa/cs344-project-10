#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define PAGE_SHIFT 8  // Shift page number this much
#define PAGE_SIZE 256 // MUST equal 2^PAGE_SHIFT
#define PAGE_COUNT 64
#define MEM_SIZE 16384 // MUST equal PAGE_SIZE * PAGE_COUNT
#define PTP_OFFSET 64  // How far offset in page 0 is the page table pointer table

// Simulated RAM
unsigned char mem[MEM_SIZE];

int get_address(int page, int offset);
void initialize_mem(void);
unsigned char get_page_table(int proc_num);
void new_process(int proc_num, int page_count);
void print_page_free_map(void);
void print_page_table(int proc_num);