/* map the ELF into memory
 * first, find and open the file from fs
 * then, parse the file and create link map on heap
 * finally, map it into memory
 * 
 * 
 */

#include <stdlib.h>
#include <string.h>
#include "headers/NFlink.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>

/* helper to get an aligned address */
#define ALIGN_DOWN(base, size)	((base) & -((__typeof__ (base)) (size)))
#define ALIGN_UP(base, size)	ALIGN_DOWN ((base) + (size) - 1, (size))

struct filebuf
{
    long len; //store the length of bytes in buf
    char buf[832] __attribute__ ((aligned (__alignof(Elf64_Ehdr))));
    //size==832 is a heuristic in dl-load.c which will meet the requirement of most ELFs
    //note that buf is exclusively for ELF header, even at the end of it may exist some fragments of phdr, we don't care
};

/* struct to store PT_LOAD info */
struct loadcmd
{
  Elf64_Addr mapstart, mapend, dataend, allocend;
  Elf64_Off mapoff;
  int prot;                             /* PROT_* bits.  */
};

struct NF_link_map *NF_map(const char *file, int mode, void *addr)
{
    int fd;

    /* TODO: actually serach directories here */
    fd = open(file, O_RDONLY, 0);
    /* it may help to know the SONAME and REALNAME of a so */

    struct filebuf fb;

    /* just cram into buf as much as you can */
    /* use a do-while can assure this event won't be interrupted */
    size_t retlen = read(fd, fb.buf, sizeof(fb.buf) - fb.len);

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)fb.buf;
    size_t maplength = ehdr->e_phnum * sizeof(Elf64_Phdr);

    /* create a new link map, and fill in header table info */
    struct NF_link_map *l = calloc(sizeof(struct NF_link_map), 1);
    l->l_phnum = ehdr->e_phnum;

    /* note that in dl-load: open_verify 
     * That function open an file with a little bit check to the phdrs
     * And the same thing just appear once again in _dl_map_object_from_fd
     */
    /* store the phdrs on heap for further loading */
    Elf64_Phdr *phdr = malloc(maplength);
    pread(fd, (void *)phdr, maplength, ehdr->e_phoff);

    struct loadcmd loadcmds[l->l_phnum];
    int nloadcmds = 0;
    bool has_holes = false;

    Elf64_Phdr *ph;
    /* travese the segments! */
    for(ph = phdr; ph < &phdr[l->l_phnum]; ++ph)
    {
        switch (ph->p_type)
        {
        case PT_LOAD:
            /* load and add it to loadcmds */
            struct loadcmd *c = &loadcmds[nloadcmds++]; //get the current loaction in loadcmds
            /* TODO: pagesize are set to 4096 for simplicity */
            c->mapstart = ALIGN_DOWN (ph->p_vaddr, 4096);
	        c->mapend = ALIGN_UP (ph->p_vaddr + ph->p_filesz, 4096);
	        c->dataend = ph->p_vaddr + ph->p_filesz;
	        c->allocend = ph->p_vaddr + ph->p_memsz;
	        c->mapoff = ALIGN_DOWN (ph->p_offset, 4096);

            /* hey ELF, is it your last segment? */
            if(nloadcmds > 1 && c[-1].mapend != c->mapstart)
                has_holes = true;

            /* setting the protection of this segment. may have errors */
            c->prot = 0;
            c->prot |= ph->p_flags & PROT_READ;
            c->prot |= ph->p_flags & PROT_WRITE;
            c->prot |= ph->p_flags & PROT_EXEC;

            break;
        
        default:
            break;
        }
    }
}