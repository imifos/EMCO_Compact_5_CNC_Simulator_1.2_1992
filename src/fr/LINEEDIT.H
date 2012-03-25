/*  Unit LINEEDIT ermoeglicht des Editieren einer einzelnen Textzeile.
 *  Alle Parameter werden in der PARAMETERSTRUCTUR definiert un muessen
 *  alle initialisiert werden. (Ausser Mausvariabeln)
 *  Maus wird nicht unterstuetzt.
 *
 *  3.92
 */

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <graph.h>

#ifndef LINEEDIT_H
#define LINEEDIT_H

/* Return codes */
#define LE_OK		  (-100)
#define LE_ESC		  (-1)
#define LE_OBEN_RAUS	  (-2)
#define LE_UNTEN_RAUS	  (-3)
#define LE_RECHTS_RAUS	  (-4)
#define LE_LINKS_RAUS	  (-5)
#define LE_MAUSKLICK_RAUS (-6)
#define LE_MENUE_RAUS	  (-7)

#define LE_CTRL_Y   (-8)
#define LE_CTRL_RET (-9)
#define LE_CTRL_INS (-10)
#define LE_CTRL_PD  (-11)
#define LE_CTRL_PU  (-12)
#define LE_FUN_1    (-13)
#define LE_FUN_12   (-14)

/* Parameteruebergabestructur */
struct LE_parameter {
		      char *editstr;	   /* String, der editiert wird.    */
		      char *zeichenmenge;  /* Menge aller erlaubten Zeichen */
		      char x,y; 	   /* X Y- Pos. des Eingabefensters */
		      char begin_xpos;	   /* Cursorposition im Fenster beim*/
					   /* Start des Editierens	    */
		      char erstetaste;	   /* Wert wird als 1. Tastendruck  */
					   /* angesehen.		    */

		      /* Flags */
		      char raus_oben,	   /* Ausstieg nach oben,	    */
			   raus_unten,	   /*		    unten,	    */
			   raus_rechts,    /*		    rechts,	    */
			   raus_links,	   /*		    links,	    */
			   raus_mausklick, /*	      durch Mausklick,	    */
			   esc_erlaubt,    /*		    ESC		    */
					   /* erlaubt.			    */
			   raus_ueberlauf, /* Ausstieg wenn beim Schreiben  */
					   /* rechte Feldgrenze erreicht    */
					   /* wird.			    */
			   insert;	   /* Beginn: insert ein oder aus   */
		    };

/* Prototypen */
char LE_LineEdit(struct LE_parameter*);
void XLE_outchar(char c);		 /* <-- Interne Function */

/*== Externe Variabeln ==*/
extern char LE_Ret_MausXPos	  ;  /*Bei Klick-Ausstieg, Rueckgabe der*/
extern char LE_Ret_MausYPos	  ;  /*Koordinaten .			  */
extern char LE_Ret_letztes_Zeichen;  /*Beinhaltet immer das letzte Zeichen*/
extern char LE_Ret_XPos		  ;  /*Beinh. die letzte Cursorposition   */


#endif
