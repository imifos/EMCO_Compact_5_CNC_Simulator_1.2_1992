/* Routine ermoeglicht die Steuerung eines Balkenmenues mit Cursortasten
 * Maus wird nicht unterstuetzt.
 * Return: ESC => BM_ESC (=99), sonst gewaehlter Punkt. (0 1 2...)
 *
 * V2.0 1/3/91
 */

#include <stdio.h>
#include <graph.h>
#include <string.h>
#include <conio.h>


#ifndef BALKENM_H
#define BALKENM_H

#define BM_ESC 99   /* Returnwert bei ESC */
#define BM_NOKEY 1  /* Wenn 1/2-Taste-Parameter nicht erwuenscht */



struct BM_parameter {
		      char *balken;
		      /* Zeiger auf Menuestring: (Trennzeichen |)
			 nur in "|" eingeschlossene Punkte koennen
			 gewaehlt werden. Anzahl von | muss gerade
			 sein. Das letzte Zeichen im String darf
			 NICHT "|" sein.
		      */

		      /* Farben */
		      long paper;	  /* Hintergrund	*/
		      short pen;	  /* Text		*/
		      long invers_paper;  /* Hintergrund Cursor */
		      short invers_pen;   /* Text Cursor	*/

		      /* Flags 1=TRUE 0=FALSE */
		      char raus_rauf,	      /* Pfeil rauf=ENTER   */
			   raus_runter,       /* Pfeil runter=ENTER */
			   esc_erlaubt;       /* ESC erlaubt	    */

		      /* Starteinstellungen */
		      char erste_position;  /* Feld auf dem der Cursor	*/
					    /* zu Beginn steht		*/
		      char xpos,ypos;	    /* X- & Y-Koord. des Balken */
		      char taste1;	    /* Wert wird als 1. Taste	*/
					    /* nach Aufruf aufgefasst.	*/
		      char taste2;	    /* Wert wird als 2. Taste	*/
					    /* nach Aufruf aufgefasst.	*/
		      char taste3;
		    };


/*+++ Hauptroutine +++*/
char BM_Menue(struct BM_parameter*);

#define BM_READ 100
char BM_letzte_wahl(char);
/*| Enthaelt die Nummer des letzten gewaehlten Punktes (Para:BM_READ)
  | Kontroll-Returnwerte werden nicht gespeichert
  | Diese Variable muss vom Benutzer zurueckgesetzt werden (Para:Wert)
 */

/*+++ Unterroutinen +++*/
void XBM_outchar(char);
void XBM_SetzteCursor(char,char*,struct BM_parameter*);
void XBM_LoescheCursor(char,char *,struct BM_parameter*);


#endif
