#
# Watcom make file for making the WMS libraries.
#

# Define the dependents order !

.EXTENSIONS:
.EXTENSIONS: .exe .obj .asm .c .cpp .res .rc

%INCLUDE +=;..\include;..\..\include

OBJS = anchor.obj habmap.obj bw.obj hwndmap.obj paintq.obj cpi.obj hdcmap.obj &
	capture.obj focus.obj hwmou.obj paint_dp.obj region.obj sysvalue.obj &
 	timestmp.obj wmsdt.obj wmstimer.obj hmqmap.obj msgqueue.obj &
	hwkeybrd.obj hwmouse.obj sqe_dp.obj sysqueue.obj wms.obj wmsmutex.obj

.obj: $(%TEMP)
.cpp: ..\source\anchor;..\source\bw;..\source\cpi;..\source\misc;
.cpp: ..\source\msgqueue;..\source\sysq;..\source

.cpp.obj:
	@echo Compiling : $*.cpp
	@wpp -fo=$(%TEMP)\$* -s -dBIF_IN_SLL -zq -ox -5 -d2 -bt=dos -ml $[*

ALL : wms.lib

anchor.obj 	: ..\source\anchor\anchor.cpp
habmap.obj 	: ..\source\anchor\habmap.cpp
bw.obj 		: ..\source\bw\bw.cpp
hwndmap.obj 	: ..\source\bw\hwndmap.cpp
paintq.obj 	: ..\source\bw\paintq.cpp
cpi.obj 	: ..\source\cpi\cpi.cpp
hdcmap.obj 	: ..\source\cpi\hdcmap.cpp
capture.obj 	: ..\source\misc\capture.cpp
focus.obj 	: ..\source\misc\focus.cpp
hwmou.obj 	: ..\source\misc\hwmou.cpp
paint_dp.obj 	: ..\source\misc\paint_dp.cpp
region.obj 	: ..\source\misc\region.cpp
sysvalue.obj 	: ..\source\misc\sysvalue.cpp
timestmp.obj 	: ..\source\misc\timestmp.cpp
wmsdt.obj 	: ..\source\misc\wmsdt.cpp
wmstimer.obj	: ..\source\misc\wmstimer.cpp
hmqmap.obj 	: ..\source\msgqueue\hmqmap.cpp
hwkeybrd.obj 	: ..\source\sysq\hwkeybrd.cpp
hwmouse.obj 	: ..\source\sysq\hwmouse.cpp
sqe_dp.obj 	: ..\source\sysq\sqe_dp.cpp
sysqueue.obj 	: ..\source\sysq\sysqueue.cpp
msgqueue.obj 	: ..\source\msgqueue\msgqueue.cpp
wms.obj 	: ..\source\wms.cpp
wmsmutex.obj 	: ..\source\wmsmutex.cpp

wms.lib : $(OBJS)
	@for %i in ($(OBJS)) do @%append wms.lb1 +$(%TEMP)\%i
	@echo Making library ...
 	@wlib -b -c -n -q -p=512 wms.lib @wms.lb1
	@del wms.lb1

cleanup : .SYMBOLIC
	@for %i in ($(OBJS)) do @del $(%TEMP)\%i