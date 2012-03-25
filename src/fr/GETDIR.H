#ifndef GETDIR_H
#define GETDIR_H

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <direct.h>

#include "tfenster.h"
#include "system.h"

/* PROTOTYPEN */

void DIR_Make	   (char*,unsigned);
/* Dir einlesen:
   1: Suchstring ("*.*")
   2: Attribut (C-Constanten siehe HELP:_dos_findfirst) */

void DIR_ReadNr	   (short,struct DIR_rec*);
/* Lesen eines bestimmten Element (para 1) in die Structur para 2 */

void DIR_ReleaseAll (void);
/* Fuer Dir reservierten Speicher loesen */

char DIR_Wahl(char,char,char*, char*,char*,char*);
/* XP,YP,SUCHSTRING, Zeiger auf Retstr: fuer name,fuer pfad,fuer drive */

struct DIR_rec
       {
	 char name[20];
	 long laenge;
	 char datum[10];
	 char dirident;
	 char driveident;

	 struct DIR_rec _far *vorher;
       };

extern short DIR_eintraege; /* Anzahl Dir-Eintraege */

#endif
