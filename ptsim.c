#include "ptsim.h"

/**
 * Convert a page, offset into an address
 */
int get_address(int page, int offset)
{
    // return page * PAGE_SIZE + offset // is this the same thing as bitshifting below?
    return (page << PAGE_SHIFT) | offset;
}

/**
 * Initialize RAM
 */
void initialize_mem(void)
{
    memset(mem, 0, MEM_SIZE); // zero every byte in the mem array

    int zpfree_addr = get_address(0, 0);
    mem[zpfree_addr] = 1; // mark reserved zero page as allocated
}

/**
 * Get the page table page for a given process
 */
unsigned char get_page_table(int proc_num)
{
    int ptp_addr = get_address(0, PTP_OFFSET + proc_num);
    return mem[ptp_addr];
}

/**
 * Return the index of the first freely available page in memory.
 */
int allocate_page(void)
{
    for (int i = 0; i < PAGE_COUNT; i++)
    {
        if (mem[i] == 0)
        {
            mem[i] = 1;
            return i;
        }
    }
    return 0xff; // sorry, no free pages
}

/**
 * Allocate pages for a new process,
 * this includes the new process page table and page_count data pages.
 */
void new_process(int proc_num, int page_count)
{
    // get the page table page
    int page_table = allocate_page();

    if (page_table == 0xff) // initial page table allocation failed
    {
        printf("OOM: proc %d: page table\n", proc_num);
        return;
    }

    // set this process's page table pointer in zero page
    mem[64 + proc_num] = page_table;

    // allocate data pages
    for (int i = 0; i < page_count; i++)
    {
        // physical page number
        int new_page = allocate_page();

        if (new_page == 0xff) // page allocation failed
        {
            printf("OOM: proc %d: data page\n", proc_num);
            return;
        }

        // set page table to map virtual to physical
        int pt_address = get_address(page_table, i);
        mem[pt_address] = new_page;
    }
}

/**
 * Print the free page map (don't modify)
 *
 */
void print_page_free_map(void)
{
    printf("--- PAGE FREE MAP ---\n");

    for (int i = 0; i < 64; i++)
    {
        int addr = get_address(0, i);

        printf("%c", mem[addr] == 0 ? '.' : '#');

        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
}

/**
 * Print the address map from virtual pages to physical (don't modify)
 */
void print_page_table(int proc_num)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", proc_num);

    // Get the page table for this process
    int page_table = get_page_table(proc_num);

    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++)
    {
        int addr = get_address(page_table, i);

        int page = mem[addr];

        if (page != 0)
        {
            printf("%02x -> %02x\n", i, page);
        }
    }
}

/**
 * Entrypoint -- process command line
 */
int main(int argc, char *argv[])
{
    assert(PAGE_COUNT * PAGE_SIZE == MEM_SIZE);

    if (argc == 1)
    {
        fprintf(stderr, "usage: ptsim commands\n");
        return 1;
    }

    initialize_mem();

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "pfm") == 0)
        {
            print_page_free_map();
        }
        else if (strcmp(argv[i], "ppt") == 0)
        {
            int proc_num = atoi(argv[++i]);
            print_page_table(proc_num);
        }
        else if (strcmp(argv[i], "np") == 0)
        {
            // launch a new process n with an initial allocation of m pages
            new_process(
                atoi(argv[i + 1]),
                atoi(argv[i + 2]));
        }
    }
    return 0;
}
