/* TFENSTER.C
   Fensterroutinen fuer den Textmodus
   V2.92 
*/

#ifndef TFENSTER_H
#define TFENSTER_H

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <malloc.h>
#include <string.h>
#include <graph.h>
/* #include "tmaus.h" */

#define TW_MAXWINDOWS	5
#define TW_NO_TITEL	0
#define TW_TITEL_OBEN	1
#define TW_TITEL_UNTEN	2
#define TW_TITEL_RECHTS 3
#define TW_TITEL_LINKS	4
#define TW_RAHMEN_NON	0
#define TW_RAHMEN_S1	1
#define TW_RAHMEN_S2	2
#define TW_RAHMEN_D1	5
#define TW_RAHMEN_D2	6

/* PROTOTYPEN & VARIABELN*/

char TW_Open_Window (char x,char y,char br,char ho,char rahmen,
		     char titelart,char ofs,char *titel);
void TW_Close_Window(void);
void TW_Next_Window (void);
void TW_Prev_Windows(void);

#endif
