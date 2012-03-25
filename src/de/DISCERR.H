/*
   Functionen fangen eines Diskettefehler (& Meldung) ab und geben
   eine neue Fehlermeldung aus.
   Bei Programmstart muss eine Initialisierung erfolgen, die den Interr.
   24h (Critical Dos-Error-Handel) umbiegt.
   Das Zurueckpatchen erfolgt bei Programmende automatisch durch DOS (alte
   Adresse befindet sich im PSP).
 */

#ifndef DISCERR_H
#define DISCERR_H

#include <dos.h>
#include <graph.h>
#include <direct.h>
#include "pdmenu.h"
#include "tfenster.h"
#include "system.h"

#define DE_EXIST   1
#define DE_NOEXIST 0
#define DE_ERROR   (-1)
#define DE_NOERROR (-2)
#define DE_REPEAT  (-3)
#define DE_ESC	   (-4)

void DE_Init(void);
void DE_HoleReg(int *ax,int *di);  /* Diese Routine draft NICHT gleichzeitig*/
				   /* mit DE_DISKERROR benutzt werden,	    */
				   /* entweder - oder			    */
char DE_DiskError(FILE*,char*);
/*		   ³	 ³
		   ³	 Filename
		   Zeiger auf File (Error wenn NULL)
 */

/* Return: 1 wenn File existent, 0 wenn nicht, DE_ESC wenn Abbruch */
char DE_FileExit(char*);

void DE_ResetErrorMemory(void);
#endif
