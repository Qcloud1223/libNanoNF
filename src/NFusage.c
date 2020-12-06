/* return the memory usage of a shared object */
#define _GNU_SOURCE //for mempcpy
#include "../NFlink.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> //for syscall open
#include <stdlib.h>
#include <elf.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

struct NF_list *head = NULL, *tail = NULL;
uint64_t dissect_and_calculate(struct NF_list *nl);
#define ALIGN_DOWN(base, size)	((base) & -((__typeof__ (base)) (size)))
#define ALIGN_UP(base, size)	ALIGN_DOWN ((base) + (size) - 1, (size))
static const char *sys_path[2];

void init_system_path()
{
    sys_path[0] = "/usr/lib/x86_64-linux-gnu/";
    sys_path[1] = "/lib/x86_64-linux-gnu/";
}

uint64_t NFusage(void *ll)
{
    /* do simple math, this is filled when NFopening */
    struct NF_link_map *l = ll;
    return l->l_map_end - l->l_map_start;
}

uint64_t NFusage_worker(const char *name, int mode)
{
    /* examine the full dependency list, and calculate the space needed
     * look into the header of everything on the list
     * The list is declared by the head and tail pointer. In this function
     * the list is built, and later it is used to do the reloc for everything
     * before NFopen returns, the list should be freed.
     */
    int fd = open(name, O_RDONLY);
    if(fd == -1)
    {
        //check the DT_RUNPATH of current map
        printf("In mapping %s as an NF: file not found\n", name);
    }
    //set head and tail when cold start
    head = calloc(sizeof(struct NF_list), 1);
    tail = head;

    //fill in info needed for dissect
    struct filebuf fb;
    fb.len = 0;
    size_t retlen = read(fd, fb.buf, sizeof(fb.buf) - fb.len);
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)fb.buf;
    Elf64_Addr maplength = ehdr->e_phnum * sizeof(Elf64_Phdr);

    //allocate the link map early, and fill in some info
    head->map = calloc(sizeof(struct NF_link_map), 1);
    head->fd = fd;
    char *real_name = strdup(name); //don't forget to free it when destroying a link_map!
    head->map->l_name = real_name;
    head->map->l_phnum = ehdr->e_phnum;
    head->map->l_phlen = maplength;
    head->map->l_phoff = ehdr->e_phoff;

    //not useful for we're using calloc
    //head->next = NULL;
    init_system_path(); //init default search path before actually working
    struct NF_list *iter = head;
    Elf64_Addr total = 0;
    while(iter != NULL)
    {
        total += dissect_and_calculate(iter);
        iter = iter->next;
    }

    return total;
}

uint64_t dissect_and_calculate(struct NF_list *nl)
{
    /* this function examine ``nl`` on the list, and put its dependencies on the list */
    //fb.buf is used as a pointer to ELF header, later we will do this again in building up the link_map
    struct NF_link_map *l = nl->map;
    Elf64_Phdr *phdr = malloc(l->l_phlen);
    pread(nl->fd, (void *)phdr, l->l_phlen, l->l_phoff);
    uint16_t phnum = l->l_phnum;

    Elf64_Dyn *ld;
    Elf64_Addr mapstart = -1, mapend;

    /* now dealing with the program headers, from which we need to 
       access the PT_LOAD for memory it takes up, and 
       PT_DYNAMIC for the contents of .dynamic */
    for(Elf64_Phdr *ph = phdr;ph < &phdr[phnum]; ++ph)
    {
        if(ph->p_type == PT_LOAD)
        {
            if(mapstart < 0)
                mapstart = ALIGN_DOWN(ph->p_vaddr, 4096);
            mapend = ph->p_vaddr + ph->p_memsz;
        }
        else if(ph->p_type == PT_DYNAMIC)
        {
            ld = malloc(ph->p_memsz);
            /* though it is not the same time, but it is the same fd... */
            pread(nl->fd, (void *)ld, ph->p_memsz, ph->p_offset); //note that offset and paddr are not the same
        }
    }

    Elf64_Dyn *str, *dyn = ld, *curr = ld;
    int nrunp = 0, nneeded = 0;
    
    while(curr->d_tag != DT_NULL)
    {
        switch(curr->d_tag)
        {
            case DT_STRTAB:
                str = curr;
                break;
            case DT_NEEDED:
                nneeded++;
                break;
            case DT_RUNPATH:
                nrunp++;
                break;
        }
        curr++;
    }
    l->l_runpath = (const char **)malloc(nrunp * sizeof(char *));
    l->l_search_list = (struct NF_link_map **)calloc(nneeded + 2, sizeof(struct NF_link_map*));
    int runpcnt = 0;
    int neededcnt = 0; //again, this is for filling the search list

    while (dyn->d_tag != DT_NULL)
    {
        if(dyn->d_tag == DT_RUNPATH)
        {
            char *tmp_name = malloc(64);
            pread(nl->fd, tmp_name, 64, str->d_un.d_ptr + dyn->d_un.d_val);
            l->l_runpath[runpcnt] = tmp_name;
            runpcnt++;
        }
        dyn++;
    }
    
    dyn = ld;
    while(dyn->d_tag != DT_NULL)
    {
        if(dyn->d_tag == DT_NEEDED)
        {
            neededcnt = 0; //neededcnt is DT_NEEDED-wise
            Elf64_Addr offset = dyn->d_un.d_val;
            char *filename = malloc(128); //store the filename for each dependency
            pread(nl->fd, filename, 128, str->d_un.d_ptr + offset); //change from 64 to 128 because the prefix is long
            struct NF_list *tmp = head;
            int found = 0;
            while(tmp != NULL)
            {
                if(!strcmp(filename, tmp->map->l_name))
                {
                    found = 1;
                    nl->map->l_search_list[neededcnt++] = tmp->map;
                    break;
                }
                tmp = tmp->next;
            }
            if(!found)
            {
                int fd = open(filename, O_RDONLY);
                /* XXX problems here
                 * open will successfully deal with file under the current directory
                 * but dlopen won't until RUNPATH is set
                 */
                if(fd == -1)
                {
                    //XXX: THIS IS NOT REALLY USEABLE NOW, PLZ ADD RUNPATH SUPPORT
                    //only lib1.so -> lib2.so -> lib3.so will work now ... and no libc func can be used
                    
                    char buf[128];
                    for(int i = 0;i < runpcnt; i++)
                    {
                        char *ptr = mempcpy(buf, l->l_runpath[i], strlen(l->l_runpath[i]));
                        *ptr = '/'; //add a "/" to make it a path
                        ptr++;
                        memcpy(ptr, filename, strlen(filename) + 1); //+1 to copy \0. memcpy only copy num bytes
                        if((fd = open(buf, O_RDONLY)) != -1)
                            break;
                    }
                    //nothing found in runpath, now try system path
                    if(fd == -1)
                    {
                        for(int i = 0; i< 2;i++)
                        {
                            char *ptr = mempcpy(buf, sys_path[i], strlen(sys_path[i]));
                            memcpy(ptr, filename, strlen(filename) + 1);
                            if((fd = open(buf, O_RDONLY)) != -1)
                                break;
                        }
                        if(fd == -1)
                            printf("In mapping %s as a dependency: file not found\n", filename);
                    }
                    
                }
                struct NF_list *tmp = calloc(sizeof(struct NF_list), 1);
                //reset the pointers to fit a new element
                tail->next = tmp;
                tmp->next = NULL;
                tail = tmp; //set tail to be the last unique element
                /* filling the info all over again */
                struct filebuf fb;
                fb.len = 0;
                size_t retlen = read(fd, fb.buf, sizeof(fb.buf) - fb.len);
                Elf64_Ehdr *ehdr = (Elf64_Ehdr *)fb.buf;
                Elf64_Addr maplength = ehdr->e_phnum * sizeof(Elf64_Phdr);

                tmp->map = calloc(sizeof(struct NF_link_map), 1);
                tmp->fd = fd;
                char *real_name = strdup(filename); //don't forget to free it when destroying a link_map!
                tmp->map->l_name = real_name;
                tmp->map->l_phnum = ehdr->e_phnum;
                tmp->map->l_phlen = maplength;
                tmp->map->l_phoff = ehdr->e_phoff;
                l->l_search_list[neededcnt++] = tmp->map;
            }
        }
        dyn++;
    }
    l->l_search_list[neededcnt] = l; //quick fix for global variables to relocate

    nl->len = ALIGN_UP(mapend - mapstart, 4096); //use this to get the exact location for next so if we want it compact
    return ALIGN_UP(mapend - mapstart, 4096);
}