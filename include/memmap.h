/* memmap.h — where the hand-placed blocks live.
 *
 * The linker places code, rodata and bss upwards from CRT_ORG_CODE and
 * knows nothing about anything below.  Everything ABOVE the program is
 * placed by hand here, because the linker must not put code there: on a
 * 128K-class machine 0xC000-0xFFFF is a paged bank, and anything the
 * linker leaves up there vanishes the moment something pages.
 *
 * tools/checkmem.py parses this file rather than duplicating it, and
 * fails the build if the linker crosses 0xC000.
 *
 * The hello-world app needs nothing up here yet.  Add blocks as
 *
 *     #define MEM_FOO   (MEM_END_OF_PREVIOUS + SIZE_OF_PREVIOUS)
 *
 * keeping MEM_END last, and `make map` will show what is left.
 */
#ifndef _MEMMAP_H_
#define _MEMMAP_H_

/* 0xC000-0xDAFF is the 128K shadow screen (RAM page 7).  Start above it
   so the same addresses are usable on both machines. */
#define MEM_BASE        0xDB00

#define MEM_END         MEM_BASE

#if MEM_END > 0x10000
#error "the hand-placed blocks run off the top of RAM"
#endif

#endif /* _MEMMAP_H_ */
