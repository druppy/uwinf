#
#	Makefile for Watcom C/C++ 10.6, for both MS-Windows and OS/2
#
#	Folowing settings is vadid
#
#	DEBUG 	: Set this to 1 if you need a debug library
#	THREAD	: Include the tread library, and make it threaded too ?
#	ASDLL	: Make the library as a DLL
#
# Target OS
#	WIN16   : Make a Windows 3.1 16 bit compatible library
#	WIN32	: Make a Windows NT/95 32 bit compatible library
#	DOS	: Dos real mode, text mode.
#	DOS32	: Dos 32 bit, text mode.
#	OS2PM 	: Make a OS2 PM (WPS) 32 bit library
#	OS2TXT	: Make a OS2 32 bit text mode library.
#
#	clear   : will cleanup a library object file set !
#
#	The library name : BCCCSSTD/I
#
#	CCC = Compiler type (W10 = Watcom 10.x)
#	SS = System type (16 bit og 32 bit)
#	T = Multi thread compilation
#	D = Debug
#	I = Import library for DLL
#
#  Still missing the Thread + mix + debug !!!
#

# Define the dependents order !
.EXTENSIONS:
.EXTENSIONS: .exe .obj .asm .c .cpp .res .rc
.cpp: ..\source;

!ifndef DEBUG
!  define DEBUG 0
!endif

# Define the temporary storage place
!ifdef %TEMP
!  define OUT $(%TEMP)
!else
!  Error, missing the TEMP enviroment variable !
!endif

# Debug enviroment setup

!ifneq DEBUG 0
!  define CGENOPT -d2 -s -bm -zm -zq -zc -DBUILDING_BIF -I..\include
!  define LGENOPT debug all option eliminate
!else
!  define CGENOPT -DNDEBUG -oxs -bm -zm -zq -zc -DBUILDING_BIF -I..\include
!  define LGENOPT option eliminate
!endif

###############################
# Object files to create !!!

# Globale log and debug elements
DBGOBJS = dbglog.obj log.obj

# Windows 16 bit (Windows 3.x)
W16OBJS = applicat.obj wndcore.obj

# Windows 32 bit (NT/95)
W32OBJS = applicat.obj wndcore.obj

# Windows common code
WINCOBJS = appwnd.obj checkbox.obj clhndl.obj combobox.obj comhndl.obj &
	control.obj csi.obj dialog.obj filedlg.obj frame.obj groupbox.obj &
	keyboard.obj listbox.obj mle.obj mouse.obj msgbox.obj ownerdrw.obj &
	pushbut.obj radiobut.obj scroll.obj sizerest.obj sle.obj slider.obj &
	slider31.obj stext.obj wndmisc.obj

# OS/2 PM
PMOBJS = applicat.obj appwnd.obj checkbox.obj clhndl.obj combobox.obj &
	comhndl.obj control.obj csi.obj dialog.obj filedlg.obj frame.obj &
	groupbox.obj keyboard.obj listbox.obj mle.obj module.obj mouse.obj &
	msgbox.obj ownerdrw.obj pushbut.obj radiobut.obj scroll.obj &
	sizerest.obj sle.obj slider.obj stext.obj wndcore.obj wndmisc.obj

# Common text mode
OTXTOBJS = applicat.obj appwnd.obj bwclpbrd.obj bwwdt.obj checkbox.obj &
	clhndl.obj comhndl.obj control.obj dialog.obj dynmsg.obj frame.obj &
	groupbox.obj keyboard.obj lowlbut.obj module.obj mouse.obj &
	msgbox.obj presparm.obj pushbut.obj radiobut.obj restempl.obj &
	scrollba.obj sle.obj stext.obj trackrct.obj wndcore.obj	wndmisc.obj &
	wndprop.obj

###################
# Thread objects

# Windows 16 treads.
TW16OBJS = btmsgbox.obj thread3.obj mwthread.obj wthread.obj mtapp.obj &
	wmwthrd.obj

# Win 32 threads.
TW32OBJS = bifmain.obj eventsem.obj mthread.obj mutexsem.obj &
	semaphor.obj spinsem.obj thread1.obj thread2.obj thread3.obj &
	thread4.obj thread5.obj threadma.obj mwthread.obj wthread.obj &
        bwwinth.obj mtappmwt.obj

# Pure Dos32/16 thread.
TDOSOBJS = dosyield.obj thread3.obj mwthread.obj wthread.obj bwwinth.obj &
	mtappmwt.obj bwwinth.obj mtappmwt.obj

# Win16 + Dos16/32 common thread.
TCOMOBJS = biftmain.obj event.obj mthread.obj mutex.obj semaphor.obj &
	spinsem.obj thread.obj thread1.obj thread2.obj thread4.obj &
	threadma.obj waitthin.obj bwwinth.obj mtappmwt.obj

# OS/2 thread.
TOS2OBJS = bifmain.obj eventsem.obj mthread.obj mutexsem.obj &
	semaphor.obj spinsem.obj thread1.obj thread2.obj thread3.obj &
	thread4.obj thread5.obj threadma.obj mwthread.obj wthread.obj &
	bwwinth.obj mtappmwt.obj

# System compiler setup

.obj: $(OUT)	# All OBJ files are placed in temp, so look there for them !

!ifdef WIN16
!  ifndef MAKETHREAD
!    define MAKETHREAD 0
!  endif

.cpp: ..\source\window\win16;..\source\window\winc;..\source\thread\win16
!  ifdef ASDLL
!    define LOPT system Windows dll option map,sym,many
!    define COPT -5 -bt=Windows -DBIF_IN_DLL -bd
!    define LIBRARY WIN16\BW1016I.LIB
!    define DLLNAME WIN16\BW1016.DLL
!  else
!    define COPT -s -5 -ml -bt=Windows -DSTRICT
!    define LOPT system win16
!    ifneq DEBUG 0
!      define LIBRARY WIN16\BW1016D.LIB
!    else
!      define LIBRARY WIN16\BW1016.LIB
!    endif
!  endif

!  ifneq MAKETHREAD 0
!    define OBJS $(W16OBJS) $(WINCOBJS) $(TW16OBJS)
!  else
!    define OBJS $(W16OBJS) $(WINCOBJS)
!  endif
!endif

!ifdef WIN32
!  ifndef MAKETHREAD
!    define MAKETHREAD 1
!  endif

!  define USE386
.cpp: ..\source\window\win32;..\source\window\winc;..\source\thread\win32;..\source\thread\win32\win;..\source\mix\win32\win
!  ifdef ASDLL
!    define LOPT system nt_dll option map,symf,many
!    define COPT -5r -bt=nt -DBIF_IN_DLL -bd -DSTRICT -DWIN32_LEAN_AND_MEAN
!    define LIBRARY WIN32\BW1032I.LIB
!    define DLLNAME WIN32\BW1032.DLL
!  else
!    define COPT -5r -bt=nt -DSTRICT -DWIN32_LEAN_AND_MEAN
!    define LOPT system nt
!    ifneq DEBUG 0
!      define LIBRARY WIN32\BW1032D.LIB
!    else
!      define LIBRARY WIN32\BW1032.LIB
!    endif
!  endif

!  ifneq MAKETHREAD 0
!    define OBJS $(W32OBJS) $(WINCOBJS) $(TW32OBJS)
!  else
!    define OBJS $(W32OBJS) $(WINCOBJS)
!  endif
!endif

!ifdef OS2PM
!  ifndef MAKETHREAD
!    define MAKETHREAD 1
!  endif

!  define USE386
.cpp: ..\source\window\pm;..\source\thread\os2;..\source\thread\os2\pm;..\source\mix\os2\pm
!  ifdef ASDLL
!    define LOPT system os2v2 dll op q option map,symf,many
!    define COPT -5r -bt=OS2 -DBIF_IN_DLL -bd
!    define LIBRARY OS2PM\BW1032TI.LIB
!    define DLLNAME OS2PM\BW1032T.DLL
!  else
!    define COPT -5r -bt=OS2
!    ifneq DEBUG 0
!      define LIBRARY OS2PM\BW1032TD.LIB
!    else
!      define LIBRARY OS2PM\BW1032T.LIB
!    endif
!  endif

!  ifneq MAKETHREAD 0
!    define OBJS $(PMOBJS) $(TOS2OBJS)
!  else
!    define OBJS $(PMOBJS)
!  endif
!endif

!ifdef OS2TXT
!  ifneq DEBUG 0
!    define LIBRARY OS2\BW1032D.LIB
!  else
!    define LIBRARY OS2\BW1032.LIB
!  endif
!  define OBJS $(OTXOBJS)
.cpp: ..\source\window\wms;..\source\tread\os2
!  define COPT -5r -bt=OS2
!  define LOPT system os2v2
!  define MAKETHREAD 1     #
!  define WMS
!  define OS2
!endif

# Make the project

!ifndef LIBRARY
!  error Missing some parameters, try again !!!
!endif

.cpp.obj:
	@echo Compiling : $[*
!ifdef USE386
	@wpp386 -fo=$(OUT)\$^@ $(CGENOPT) $(COPT) $[*
!else
	@wpp -fo=$(OUT)\$^@ $(CGENOPT) $(COPT) $[*
!endif

.c.obj:
	@echo Compiling : $[*
!ifdef USE386
	@wcc386 -fo=$(OUT)\$^@ $(CGENOPT) $(COPT) $[*
!else
	@wcc -fo=$(OUT)\$^@ $(CGENOPT) $(COPT) $[*
!endif

# Just add debuging !

OBJS += $(DBGOBJS)

ALL : $(LIBRARY)

CLEAR: .SYMBOLIC
	@for %i in ($OBJS) do @del $OUT\%i
	@Echo Cleanup done ...

$OUT\MAKELIB.TMP : $(OBJS)
	@%create $^@
	@%write $^@ -q $(LIBRARY)
	@for %i in ($(OBJS)) do @%append $^@ -+$(OUT)\%i

$OUT\MAKELINK.TMP : $(OBJS)
	@%create $^@
	@%write $^@ name $(DLLNAME) $(LOPT)
	@for %i in ($(OBJS)) do @%append $^@ file $(OUT)\%i

# Link together the DLL file, but how ???

!ifdef ASDLL
$(LIBRARY) : $(OBJS) $OUT\MAKELINK.TMP
	@wlink @$OUT\MAKELINK.TMP
	@-wlib -n -b -p=16 $(LIBRARY) +$(DLLNAME)
	@del $(OUT)\MAKELINK.TMP
!else
$(LIBRARY) : $(OBJS) $OUT\MAKELIB.TMP
	@-wlib @$OUT\MAKELIB.TMP
	@del $OUT\MAKELIB.TMP
!endif

# All the build definements

# Globale log file and debug elements
dbglog.obj : ..\source\dbglog.cpp
log.obj : ..\source\log.cpp
