/* 2HRCMON.C: Routinen zum Ansprechen eines 2. Herculesmonitors im
   Textmode.
 */

#ifndef HRCMON_H
#define HRCMON_H

#include <stdio.h>
#include <dos.h>

/* PROTOTYPEN */

/* direkte Bearbeitung */
void MON2_clrscr(char,char,char,char);
void MON2_settextposition(char,char);
void MON2_outchar(char);
void MON2_outshort(char *,short);
void MON2_outtext(char _far*);
void MON2_setattribut(char);

/* BIOS-Interrupts */
char MON2_Init(void);
char MON2_Aktiviere_Adapter(char);

/* Statusvariable */
#define MON2_ERR  0
#define MON2_OK   1
#define MON2_SECOND  1
#define MON2_PRIMAER 0
extern char MON2_vorhanden;

#endif
