/* PULL-DOWN-MENUE-STEUERUNG
   Ermoeglicht die Steuerung eines vertikalen Menues.
   (Maus wird unterstuezt)

   1.1992 
 */

#ifndef PDMENU_H
#define PDMENU_H

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graph.h>
#include <string.h>
/* #include "mouse.h" */


#define PD_NEW_WRITE 1
#define PD_NOT_NEW_WRITE 0
#define PD_ESC_ENABLE 1
#define PD_ESC_DISABLE 0

#define PD_RET_ESC     (-1)
#define PD_RECHTS_RAUS (-2)
#define PD_LINKS_RAUS  (-3)

struct PD_parameter {
		      char xap,yap,
			   vorwahl,
			   neuschreiben,
			   escerlaubt,
			   links_raus,
			   rechts_raus,
			   anz,
			   strbr,
			   *array,*relpos;
		    };

char PD_Menue(struct PD_parameter*);

#endif

/* Parameter:
xap,yap     : Menueeckkoordinaten
vorwahl     : 1. Position des Balken (1. Pos = 0)
neuschreiben: Flag zeigt an, ob das PDMenue vor dem Wahlvorgang
	      neu aufgebaut wird.
escerlaubt  : Ende durch ESC erlaubt J/N: bei ESC = Returnwert
anz	    : Anzahl der Menuepunkte
strbr	    : Stringlaenge der Menuepunkte. Er MUSS GLEICH der wirklichen
	      Stringlaenge,inclusive 0-Charakter, sein.
*relpos     : Zeiger auf ein Array:gibt den Abstand der aktuellen
	      zur vorherigen Zeile an:
	      Bsp: unsigned char RELPOS[5]={0,1,1,1,2};
*array	    : Zeiger auf ein Array: Liste der Menuepunkte:
	      Bsp.:ARRAY[3][10]={"  Datei  ","  Start  "," IO Def. "};
			 ³   ³	    ³
	       Anz. Punkte   ³	    Menuepunkte: Laenge MUSS gleich
			     ³			 strbr sein!
	       Breite kann groesser als
	       strbr sein
		   (Adresse &Array[0])

Beispiel eines Aufrufes:
wahl=PD_Menue(2,1,wahl,PD_NEW_WRITE,PD_ESC_ENABLE,4,18,&TEXT[0][0],&ABST[0]);
*/
