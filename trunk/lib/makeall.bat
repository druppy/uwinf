@echo off
ECHO This command will build all BIF libraries, using the makefile for
ECHO Watcom ...
PAUSE

rem Setup the watcom enviroment
set OS2_INCLUDE=%watcom%\h\os2
set WINDOWS_INCLUDE=%watcom%\h\win
set NT_INCLUDE=%watcom%\h\nt

if not exist WIN16 md WIN16
if not exist WIN32 md WIN32
if not exist OS2PM md OS2PM

wmake /h WIN16=1
wmake /h WIN16=1 clear
wmake /h WIN16=1 DEBUG=1
wmake /h WIN16=1 DEBUG=1 clear

wmake /h WIN32=1
wmake /h WIN32=1 clear
wmake /h WIN32=1 DEBUG=1
wmake /h WIN32=1 DEBUG=1 clear
wmake /h WIN32=1 ASDLL=1
wmake /h WIN32=1 ASDLL=1 clear

wmake /h OS2PM=1
wmake /h OS2PM=1 clear
wmake /h OS2PM=1 DEBUG=1
wmake /h OS2PM=1 DEBUG=1 clear
wmake /h OS2PM=1 ASDLL=1
wmake /h OS2PM=1 ASDLL=1 clear
