#include "ptsim.h"

/**
 * Convert a page, offset into an address
 */
int get_address(int page, int offset)
{
    return (page << PAGE_SHIFT) | offset;
}

/**
 * Initialize RAM
 */
void initialize_mem(void)
{
    // zero bytes
    memset(mem, 0, MEM_SIZE);

    int zpfree_addr = get_address(0, 0);
    mem[zpfree_addr] = 1; // page zero allocated
}

/**
 * Get the page table page for a given process
 */
unsigned char get_page_table(int process_number)
{
    int ptp_addr = get_address(0, PTP_OFFSET + process_number);
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
 * Deallocate a given page.
 */
void deallocate_page(int page)
{
    mem[page] = 0;
}

/**
 * Kill a given process.
 */
void kill_process(int process_number)
{
    int page_table_page = get_page_table(process_number);

    int process_page_table = get_address(
        page_table_page,
        0);

    int bound = process_page_table + PAGE_SIZE;

    for (int i = process_page_table; i < bound; i++)
    {
        if (mem[i] != 0)
        {
            deallocate_page(
                mem[i]);
        }
    }
    deallocate_page(page_table_page);
}

/**
 * Get the physical address of a given process at some virtual address.
 */
int get_physical_address(int process_number, int virtual_address)
{
    int process_page_table = get_address(mem[process_number + PAGE_COUNT], 0);
    int virtual_page = virtual_address >> 8;
    int offset = virtual_address & 255;
    int physical_page = mem[process_page_table + virtual_page];
    return ( // physical address
        (physical_page << 8) | offset);
}

/**
 * Store a given value of some process at a specified virtual address.
 */
void store_value(int process_number, int virtual_address, int value)
{
    int physical_address = get_physical_address(process_number, virtual_address);
    mem[physical_address] = value;
    printf(
        "Store proc %d: %d => %d, value=%d\n",
        process_number,
        virtual_address,
        physical_address,
        value);
}

void load_value(int process_number, int virtual_address)
{
    int physical_address = get_physical_address(process_number, virtual_address);
    int value = mem[physical_address];
    printf(
        "Load proc %d: %d => %d, value=%d\n",
        process_number,
        virtual_address,
        physical_address,
        value);
}

/**
 * Allocate pages for a new process,
 * this includes the new process page table and page_count data pages.
 */
void new_process(int process_number, int page_count)
{
    // get the page table page
    int page_table = allocate_page();

    if (page_table == 0xff)
    {
        // when initial page table allocation fails
        printf("OOM: proc %d: page table\n", process_number);
        return;
    }

    // set this process's page table pointer in page zero
    mem[64 + process_number] = page_table;

    // allocate data pages
    for (int i = 0; i < page_count; i++)
    {
        // physical page number
        int new_page = allocate_page();

        if (new_page == 0xff)
        {
            // when page allocation fails
            printf("OOM: proc %d: data page\n", process_number);
            return;
        }

        // set page table to map virtual to physical
        int pointer_address = get_address(page_table, i);
        mem[pointer_address] = new_page;
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
        int address = get_address(0, i);

        printf("%c", mem[address] == 0 ? '.' : '#');

        if ((i + 1) % 16 == 0)
            putchar('\n');
    }
}

/**
 * Print the address map from virtual pages to physical (don't modify)
 */
void print_page_table(int process_number)
{
    printf("--- PROCESS %d PAGE TABLE ---\n", process_number);

    // Get the page table for this process
    int page_table = get_page_table(process_number);

    // Loop through, printing out used pointers
    for (int i = 0; i < PAGE_COUNT; i++)
    {
        int address = get_address(page_table, i);

        int page = mem[address];

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
            // "[p]rint [f]ree [m]ap"
            print_page_free_map();
        }
        else if (strcmp(argv[i], "ppt") == 0)
        {
            // "[p]rint [p]age [t]able"
            int process_number = atoi(argv[++i]);
            print_page_table(process_number);
        }
        else if (strcmp(argv[i], "np") == 0)
        {
            // [n]ew [p]rocess
            int process_number = atoi(argv[i + 1]);
            int pages_requested = atoi(argv[i + 2]);
            new_process(
                process_number,
                pages_requested);
        }
        else if (strcmp(argv[i], "kp") == 0)
        {
            // [k]ill [p]rocess
            int process_number = atoi(argv[i + 1]);
            kill_process(process_number);
        }
        else if (strcmp(argv[i], "sb") == 0)
        {
            // [s]tore [b]
            int process_number = atoi(argv[i + 1]);
            int virtual_address = atoi(argv[i + 2]);
            int value = atoi(argv[i + 3]);
            store_value(
                process_number,
                virtual_address,
                value);
        }
        else if (strcmp(argv[i], "lb") == 0)
        {
            // [l]oad [b]
            int process_number = atoi(argv[i + 1]);
            int virtual_address = atoi(argv[i + 2]);
            load_value(
                process_number,
                virtual_address);
        }
    }
    return 0;
}
