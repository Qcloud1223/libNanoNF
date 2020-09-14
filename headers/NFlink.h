/* data structure to store the load time information of a so
 * 
 * The design logic for this file is adding elements when it is inevitably needed
 */

#ifndef _NF_link
#define _NF_link 1

/* for uint64_t to indicate address */
#include <stdint.h>
#include <elf.h> //is this necessary?

struct NF_link_map
{
    uint64_t l_addr; //the base address of a so
    char* l_name; //the absolute path
    //the dynamic section?
    Elf64_Dyn *l_ld;
    struct NF_link_map *l_prev, *l_next;

    uint64_t l_map_start, l_map_end; //the start and end location of a so
    uint16_t l_phnum; //program header count
    uint16_t l_ldnum; //load segment count
    uint64_t l_phdr; //the address of program header table

    /* dynamic section info */
    Elf64_Dyn l_info[DT_NUM];
};


#endif

