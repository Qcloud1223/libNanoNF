/* Do relocation for a loaded elf
 * Thanks to the info in l_info, we now know many extra information, like the address of synbol table
 * 
 * Note that l_info is just a index for fast, or random access
 * In limited, user version of link_map, l_ld exists. This contains enough info for symbol lookup
 * even we are not dynamic linker.
 * 
 * Simply breaking the relocation into 2 types: mess/do not mess with the GOT
 * Former one is called relative relocation: though variables and functions inside a so is access via rip
 *      other libraries will search its symbol table 
 * Latter one is more familiar: We first find the map containing the symbol, find its actual address, 
 *      and fill the GOT 
 * 
 * readelf --relocs can show the relocation entries
*/