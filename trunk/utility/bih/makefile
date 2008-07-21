#
#	Watcom makefile for making the two different BIH compilers.
#	
#	Options :
#	OS2=1	: Make the INF format instead og HLP.
#

.EXTENSIONS:
.EXTENSIONS: .exe .obj .c .cpp

!ifndef OS2
ALL: bihc.exe mktoc.exe mbihc.exe
BIHC = bihc.obj bihcrtf.obj
MBIHC = mbihc.obj mbihcrtf.obj
!else
All: bihc.exe mbihc.exe
BIHC = bihc.obj bihcipf.obj
MBIHC = mbihc.obj mbihcipf.obj
!endif

!define OUT $(%TEMP)

!ifndef DEBUG
!  define DEBUG 0
!endif

# Debug enviroment setup
COPT = -2 -ml -bt=dos -zq -zm
LOPT = system dos option quiet

!ifeq DEBUG 1
!  define CGENOPT -d2 -s 
!  define LGENOPT debug all option eliminate
!else
!  define CGENOPT -oxs 
!  define LGENOPT option eliminate
!endif

.obj: $(OUT)

.cpp.obj:
	@echo Compiling : $[*
	@wpp -fo=$(OUT)\$^@ $(CGENOPT) $(COPT) $[*

bihc.exe : $(BIHC)
	@%write $OUT\link.lrf $(LGENOPT) $(LOPT) name bihc.exe
	@for %i in ($(BIHC)) do @%append $OUT\link.lrf file $(OUT)\%i
	@-wlink @$OUT\link.lrf
	del $OUT\link.lrf

mbihc.exe : $(MBIHC)
	@%write $OUT\link.lrf $(LGENOPT) $(LOPT) name mbihc.exe
	@for %i in ($(MBIHC)) do @%append $OUT\link.lrf file $(OUT)\%i
	@-wlink @$OUT\link.lrf
	del $OUT\link.lrf

mktoc.exe : mktoc.obj
	@wlink $(LGENOPT) name mktoc.exe $(LOPT) file $(OUT)\mktoc.obj 
