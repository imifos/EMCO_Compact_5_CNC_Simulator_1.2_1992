#include <dos.h>
#include <graph.h>

#ifndef SYSTEM_H
#define SYSTEM_H

#define SYS_JA	 1
#define SYS_NEIN 0
#define SYS_ESC  2


/* PROTOTYPEN */
void SYS_MakeFarPtr(unsigned sgm,unsigned ofs,void _far **farptr);
char SYS_DruckerStatus(char);  /* Ret:	 */
char SYS_DruckerInit(char);    /* Status */
void SYS_CursorOff(void);
void SYS_CursorOn(void);
char SYS_ColorCard(void);
void SYS_PushColor(void);
void SYS_PopColor(void);
void SYS_TextInvert(void);
char SYS_JaNein(char xp,char yp,char voreinst,char escret);
char *SYS_DelEndSpaces(char*);
char SYS_DruckerPrint(char lpt,char zeichen);
char SYS_ExistLPT(char);
char SYS_ExistCOM(char);

#endif
