/* Routinen ermoeglichen die Steuerung eines kombinierten Balken- und
   Pull-Down-Menues.
*/

#ifndef KMENUE_H
#define KMENUE_H

#include <graph.h>
#include "balkenm2.h"
#include "pdmenu.h"
#include "tfenster.h"

#define anz_pds   6
#define KM_ESC	  (-1)
#define KM_NOYPOS (-2)

struct KM_parameter {
		       /* Anzahl der im BMenue vorhandenen Punkte */
		       char anz_bm;

		       /* Flags, ob ein entsprechendes PDMenue vorhanden*/
		       char pd_vorhanden[anz_pds];

		       /* Zeiger auf die Parameterstructur des Bmenues */
		       /* Naeheres sie BALKENMENUE2-Routine	       */
		       struct BM_parameter *balken;

		       /* Zeigerfeld auf die ParamStructuren der PDMenues */
		       /* Maximale Anzahl variiert je nach Programm	  */
		       /* Naeheres sie PULLDOWNMENUE-Routine		  */
		       /*						  */
		       /* BEDINGUNGEN:					  */
		       /* Im ersten PDMenue MUSS ein Ausstieg nach	  */
		       /* Links verboten werden. Idem fuer das Letzte	  */
		       /* nach Rechts					  */
		       struct PD_parameter *pdmenue[anz_pds];
		    };

void KM_Menue(char *retx,char *rety,struct KM_parameter *para);

/*
retx: Returnwert (0,1,2,...) des Balkenmenues
rety: Returnwert (0,..	   ) des PDMenues
*/

#endif
