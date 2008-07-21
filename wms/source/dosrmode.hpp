#ifndef __DOSRMODE_HPP
#define __DOSRMODE_HPP

//Provide access to DOS real-mode memory, specifically:
//  segment 0x0040   BIOS data segment
//  segment 0xb800   CGA/EGA/VGA display memory
//Provide access to int80x86 interrupts
//  int80x86   - interrupt without segment registers
//  int86x86x  - interrupt with registers

//use BIF to detect environment
#include <bif.h>
#if (BIFOS_ == BIFOS_DOS16_)
// Direct access
#  include <dos.h>
#  define Ptr_0040_Type(type) type __far *
#  define Ptr_0040(offset) MK_FP(0x0040,offset)
#  define Ptr_b800_Type(type) type __far *
#  define Ptr_b800(offset) MK_FP(0xb800,offset)
#  define int80x86 int86
#  define int80x86x int86x
#else /*32-bit*/

# ifdef __WATCOMC__
#  include <i86.h>
// Assume DOS4G. Low 1MB is mapped to first 1MB of virtual addresses
#  define Ptr_0040_Type(type) type *
#  define Ptr_0040(offset) ((void*)(0x400+offset))
#  define Ptr_b800_Type(type) type *
#  define Ptr_b800(offset) ((void*)(0xb8000+offset))
#  define int80x86 int386
#  define int80x86x int386x
#else
#error Not yet
#endif

#endif
#endif /*__DOSRMODE_HPP*/
