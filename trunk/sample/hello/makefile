#
#	Watcom makefile for BIF windowed hello world program.
#

!ifdef OS2
COPT = -bt=os2 -5r -bm -ox -zc -zm
LOPT = system OS2V2_PM lib OS2PM\BW1032T.LIB
VER = OS/2 version
CC = wpp386
!else
COPT = -bt=windows -2 -ml -zm -s -ox -DSTRICT
LOPT = SYS WINDOWS OP QUIET,ELIMINATE LIB WIN16\BW1016.LIB
VER = Windows 3.x version
CC = wpp
!endif

All: whello.exe

.cpp.obj:
	@echo Compiling : $[*.cpp
	@$(CC) -zq $(COPT) $[*.cpp

whello.exe : whello.obj
	@echo Linking $(VER)...
	@wlink $(LOPT) name whello.exe file whello.obj

whello.obj : whello.cpp
