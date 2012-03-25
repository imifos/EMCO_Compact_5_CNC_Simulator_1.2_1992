/*
This code is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 only, as
published by the Free Software Foundation.

This code is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
version 2 for more details.

Source & README published 2012
https://github.com/imifos/EMCO_Compact_5_CNC_Simulator_1.2_1992
*/

#include "kmenue.h"
#include "balkenm2.h"

#define nicht_raus 99

/* FUNCTION */

void KM_Menue(char *retx,char *rety,struct KM_parameter *para)
{
    char ywahl,xwahl,xp,yp,hoehe,za;

    /* BM Benutztervariable zuruecksetzten */
    /*BM_letzte_wahl(0);*/
    if (*retx<0 || *retx>10) *retx=0;

    do {
	 /* Balkenmenue auf letzte Pos. ausfuehren */
	 para->balken->erste_position= /*BM_letzte_wahl(BM_READ)*/ *retx;
	 xwahl=BM_Menue(para->balken);

	 if (xwahl==BM_ESC)
	  {
	    xwahl=KM_ESC;
	    ywahl=KM_ESC;
	  }
	  else
	    if (!para->pd_vorhanden[xwahl])
	      /*= Kein entsprechendes PDMenue vorhanden -> raus =*/
	      ywahl=KM_NOYPOS;
	      else
		{ /*= PD-Menue vorhanden =*/

		  /* Windowdaten bestimmen & oeffnen */
		  xp=para->pdmenue[xwahl]->xap;
		  yp=para->pdmenue[xwahl]->yap;
		  for (hoehe=0,za=0;za<para->pdmenue[xwahl]->anz;za++)
		     hoehe+=para->pdmenue[xwahl]->relpos[za];


						  TW_Open_Window(xp,yp,(char)(para->pdmenue[xwahl]->strbr+1),
				 hoehe,TW_RAHMEN_S1,TW_NO_TITEL,1,"");

		  /* PDMenue-Koordinaten korrigieren (Windowwecke) */
		  para->pdmenue[xwahl]->xap=2;
		  para->pdmenue[xwahl]->yap=1;
		  ywahl=PD_Menue(para->pdmenue[xwahl]);

		  /* Window schliessen und PDMenueKoord. wieder berichtigen */
		  TW_Close_Window();
		  para->pdmenue[xwahl]->xap=xp;
		  para->pdmenue[xwahl]->yap=yp;

		  /* Waehrend PDMenue Cursor Rechts/Links gedrueckt */
		  if (ywahl==PD_RECHTS_RAUS)
		   {
		     ywahl=nicht_raus;
		     para->balken->taste1=0;
		     para->balken->taste2=77;
		     if (para->pd_vorhanden[xwahl+1])
		       para->balken->taste3=13;
		       else para->balken->taste3=BM_NOKEY;
		   }
		  if (ywahl==PD_LINKS_RAUS)
		   {
		     ywahl=nicht_raus;
		     para->balken->taste1=0;
		     para->balken->taste2=75;
		     if (para->pd_vorhanden[xwahl-1])
		       para->balken->taste3=13;
		       else para->balken->taste3=BM_NOKEY;
		   }

		  /* Bei ESC -> Ende */
		  if (ywahl==PD_RET_ESC)
		   {
		     xwahl=KM_ESC;
		     ywahl=KM_ESC;
		   }
		}

       } while (xwahl!=KM_ESC && ywahl==nicht_raus);

    *retx=xwahl;
    *rety=ywahl;
}
