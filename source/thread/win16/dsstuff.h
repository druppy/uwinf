#ifndef __DSSTUFF_H
#define __DSSTUFF_H

//macros for the usual problem in Windows: SS and DS
#if defined(__WATCOMC__)
//Watcom C/C++
void SetDS(unsigned newDS);
#pragma aux SetDS = \
        "push ds"   \
        "mov ds,ax" \
        parm [ax]
void SetDSToSS();
#pragma aux SetDSToSS = \
        "push ds"       \
        "push ss"       \
        "pop ds"
void RestoreDS();
#pragma aux RestoreDS = \
        "pop ds"
unsigned GetSS();
#pragma aux GetSS = \
        "mov ax,ss" \
        value [ax]

#elif defined __BORLANDC__
//Borland C++
#define SetDS(newDS) asm push ds; _AX=newDS; asm mov ds,ax
#define SetDSToSS()  asm push ds; asm push ss; asm pop ds
#define RestoreDS()  asm pop ds
#define GetSS()      _SS

#elif defined( __SC__ )
//Symantec C++
#define SetDS(newDS) { _asm push ds; _asm mov ax,newDS; _asm mov ds,ax; }
#define SetDSToSS()  { _asm push ds; _asm push ss; _asm pop ds; }
#define RestoreDS()  { _asm pop ds; }
inline unsigned GetSS( void ) { _asm mov ax,ss; }

#elif defined(_MSC_VER)
//Microsoft C
#define SetDS(newDS) { _asm push ds; _asm mov ax,newDS; _asm mov ds,ax; }
#define SetDSToSS()  { _asm push ds; push ss; pop ds; }
#define RestoreDS()  { _asm pop ds; }
inline unsigned GetSS( void ) { _asm mov ax,ss; }

#else
#error hi! make some stuff for your compiler here
#endif

#endif

