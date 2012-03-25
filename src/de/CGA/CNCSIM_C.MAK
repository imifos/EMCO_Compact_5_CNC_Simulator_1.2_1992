PROJ = CNCSIM_C
PROJFILE = CNCSIM_C.MAK
DEBUG = 0

PWBRMAKE  = pwbrmake
NMAKEBSC1  = set
NMAKEBSC2  = nmake
CC  = cl
CFLAGS_G  = /AM /W4 /BATCH
CFLAGS_D  = /Od /Zi
CFLAGS_R  = /Od
MAPFILE_D  = NUL
MAPFILE_R  = NUL
LFLAGS_G  =  /NOI /STACK:12048	/BATCH
LFLAGS_D  =  /CO /INC /FAR /NOP
LFLAGS_R  =  /EXE /FAR /NOP
LINKER	= link
ILINK  = ilink
LRF  = echo > NUL
BROWSE	= 0
CVFLAGS  =  /25 /2 /E /S

OBJS  = CNCSIM.obj SIMUL.obj ..\2HRCMON.obj ..\BALKENM2.obj ..\DISCERR.obj\
	..\DISKROUT.obj ..\EDITOR.obj ..\GETDIR.obj ..\KMENUE.obj\
	..\LINEEDIT.obj ..\OPTIONEN.obj ..\PDMENU.obj ..\SERIELL.obj\
	..\SYSTEM.obj ..\TFENSTER.obj
SBRS  = CNCSIM.sbr SIMUL.sbr ..\2HRCMON.sbr ..\BALKENM2.sbr ..\DISCERR.sbr\
	..\DISKROUT.sbr ..\EDITOR.sbr ..\GETDIR.sbr ..\KMENUE.sbr\
	..\LINEEDIT.sbr ..\OPTIONEN.sbr ..\PDMENU.sbr ..\SERIELL.sbr\
	..\SYSTEM.sbr ..\TFENSTER.sbr

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .sbr .obj .c

CNCSIM.obj : CNCSIM.C ..\CNCSIM.H ..\optionen.h ..\Editor.h ..\Diskrout.h\
	..\simul.h ..\2hrcmon.h ..\balkenm2.h ..\kmenue.h ..\lineedit.h\
	..\pdmenu.h ..\system.h ..\tfenster.h ..\discerr.h ..\getdir.h\
	..\seriell.h ..\2SCREEN.H

CNCSIM.sbr : CNCSIM.C ..\CNCSIM.H ..\optionen.h ..\Editor.h ..\Diskrout.h\
	..\simul.h ..\2hrcmon.h ..\balkenm2.h ..\kmenue.h ..\lineedit.h\
	..\pdmenu.h ..\system.h ..\tfenster.h ..\discerr.h ..\getdir.h\
	..\seriell.h ..\2SCREEN.H

SIMUL.obj : SIMUL.C ..\CNCSIM.H ..\simul.h ..\2hrcmon.h ..\balkenm2.h\
	..\kmenue.h ..\lineedit.h ..\pdmenu.h ..\system.h ..\tfenster.h\
	..\discerr.h ..\getdir.h ..\seriell.h ..\2SCREEN.H

SIMUL.sbr : SIMUL.C ..\CNCSIM.H ..\simul.h ..\2hrcmon.h ..\balkenm2.h\
	..\kmenue.h ..\lineedit.h ..\pdmenu.h ..\system.h ..\tfenster.h\
	..\discerr.h ..\getdir.h ..\seriell.h ..\2SCREEN.H

..\2HRCMON.obj : ..\2HRCMON.C ..\2hrcmon.h

..\2HRCMON.sbr : ..\2HRCMON.C ..\2hrcmon.h

..\BALKENM2.obj : ..\BALKENM2.C ..\balkenm2.h

..\BALKENM2.sbr : ..\BALKENM2.C ..\balkenm2.h

..\DISCERR.obj : ..\DISCERR.C ..\discerr.h ..\pdmenu.h ..\tfenster.h\
	..\system.h

..\DISCERR.sbr : ..\DISCERR.C ..\discerr.h ..\pdmenu.h ..\tfenster.h\
	..\system.h

..\DISKROUT.obj : ..\DISKROUT.C ..\Diskrout.h ..\Editor.h ..\CNCSIM.H\
	..\2hrcmon.h ..\balkenm2.h ..\kmenue.h ..\lineedit.h ..\pdmenu.h\
	..\system.h ..\tfenster.h ..\discerr.h ..\getdir.h ..\seriell.h\
	..\2SCREEN.H

..\DISKROUT.sbr : ..\DISKROUT.C ..\Diskrout.h ..\Editor.h ..\CNCSIM.H\
	..\2hrcmon.h ..\balkenm2.h ..\kmenue.h ..\lineedit.h ..\pdmenu.h\
	..\system.h ..\tfenster.h ..\discerr.h ..\getdir.h ..\seriell.h\
	..\2SCREEN.H

..\EDITOR.obj : ..\EDITOR.C ..\Editor.h ..\CNCSIM.H ..\simul.h ..\Diskrout.h\
	..\2hrcmon.h ..\balkenm2.h ..\kmenue.h ..\lineedit.h ..\pdmenu.h\
	..\system.h ..\tfenster.h ..\discerr.h ..\getdir.h ..\seriell.h\
	..\2SCREEN.H

..\EDITOR.sbr : ..\EDITOR.C ..\Editor.h ..\CNCSIM.H ..\simul.h ..\Diskrout.h\
	..\2hrcmon.h ..\balkenm2.h ..\kmenue.h ..\lineedit.h ..\pdmenu.h\
	..\system.h ..\tfenster.h ..\discerr.h ..\getdir.h ..\seriell.h\
	..\2SCREEN.H

..\GETDIR.obj : ..\GETDIR.C ..\getdir.h ..\discerr.h ..\tfenster.h ..\system.h\
	..\pdmenu.h

..\GETDIR.sbr : ..\GETDIR.C ..\getdir.h ..\discerr.h ..\tfenster.h ..\system.h\
	..\pdmenu.h

..\KMENUE.obj : ..\KMENUE.C ..\kmenue.h ..\balkenm2.h ..\pdmenu.h\
	..\tfenster.h

..\KMENUE.sbr : ..\KMENUE.C ..\kmenue.h ..\balkenm2.h ..\pdmenu.h\
	..\tfenster.h

..\LINEEDIT.obj : ..\LINEEDIT.C ..\lineedit.h

..\LINEEDIT.sbr : ..\LINEEDIT.C ..\lineedit.h

..\OPTIONEN.obj : ..\OPTIONEN.C ..\optionen.h ..\CNCSIM.H ..\lineedit.h\
	..\pdmenu.h ..\system.h ..\tfenster.h ..\discerr.h ..\2hrcmon.h\
	..\balkenm2.h ..\kmenue.h ..\getdir.h ..\seriell.h ..\2SCREEN.H

..\OPTIONEN.sbr : ..\OPTIONEN.C ..\optionen.h ..\CNCSIM.H ..\lineedit.h\
	..\pdmenu.h ..\system.h ..\tfenster.h ..\discerr.h ..\2hrcmon.h\
	..\balkenm2.h ..\kmenue.h ..\getdir.h ..\seriell.h ..\2SCREEN.H

..\PDMENU.obj : ..\PDMENU.C ..\pdmenu.h

..\PDMENU.sbr : ..\PDMENU.C ..\pdmenu.h

..\SERIELL.obj : ..\SERIELL.C ..\seriell.h

..\SERIELL.sbr : ..\SERIELL.C ..\seriell.h

..\SYSTEM.obj : ..\SYSTEM.C ..\system.h

..\SYSTEM.sbr : ..\SYSTEM.C ..\system.h

..\TFENSTER.obj : ..\TFENSTER.C ..\tfenster.h

..\TFENSTER.sbr : ..\TFENSTER.C ..\tfenster.h


$(PROJ).bsc : $(SBRS)
	$(PWBRMAKE) @<<
$(BRFLAGS) $(SBRS)
<<

$(PROJ).exe : $(OBJS)
!IF $(DEBUG)
	$(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_D)
$(LLIBS_G: = +^
) +
$(LLIBS_D: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_D);
<<
!ELSE
	$(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
!ENDIF
!IF $(DEBUG)
	$(ILINK) -a -e "$(LINKER) @$(PROJ).lrf" $@
!ELSE
	$(LINKER) @$(PROJ).lrf
!ENDIF


.c.sbr :
!IF $(DEBUG)
	$(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FR$@ $<
!ELSE
	$(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FR$@ $<
!ENDIF

.c.obj :
!IF $(DEBUG)
	$(CC) /c $(CFLAGS_G) $(CFLAGS_D) /Fo$@ $<
!ELSE
	$(CC) /c $(CFLAGS_G) $(CFLAGS_R) /Fo$@ $<
!ENDIF


run: $(PROJ).exe
	$(PROJ).exe $(RUNFLAGS)

debug: $(PROJ).exe
	CV $(CVFLAGS) $(PROJ).exe $(RUNFLAGS)
