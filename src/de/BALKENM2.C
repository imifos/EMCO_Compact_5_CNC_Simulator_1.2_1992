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



/* BALKENMENUE2.C
   Functionen ermoeglichen die Steuerung eines Balken-(Kopf)-Menues
*/
#include "balkenm2.h"


char BM_letzte_wahl(char wahl)
{
  static char letzte_wahl=0;

  if (wahl==BM_READ)
    return letzte_wahl;
    else letzte_wahl=wahl;
}

/*+++ Hauptroutine +++*/
char BM_Menue(struct BM_parameter *para)
{
   /* Subvariabeln */
   long oldpap=_getbkcolor();
   short oldpen=_gettextcolor();
   char stpos=0,scrpos=0,zb,taste=0;

   /* Verwaltungsvar. */
   char wahl=para->erste_position;
   char punkt_pos   [10],	    /* speichert XPos jedes Punktes */
	punkt_string[10][20];	    /* speichert jeden Punkt */
   char anz_punkte=0;		    /* Anzahl aller Punkte */


   /* Init */
   _settextcolor(para->pen);
   _setbkcolor(para->paper);
   _settextposition(para->ypos,para->xpos);

   /* Balken das erste mal Ausgeben */
   for (zb=0;zb<(char)strlen(para->balken);zb++)
     if (para->balken[zb]!='|')
       XBM_outchar(para->balken[zb]);

   /* Bestimme parameter der verschiedenen Punkte */
   while (para->balken[stpos]!='\0')
    {
      if (para->balken[stpos]=='|')
	{
	   zb=0;
	   stpos++;
	   punkt_pos[anz_punkte]=scrpos;
	   while (para->balken[stpos]!='|')
		{
		  punkt_string[anz_punkte][zb]=para->balken[stpos];
		  zb++;
		  stpos++;
		  scrpos++;
		}
	   punkt_string[anz_punkte][zb]='\0';
	   anz_punkte++;
	   scrpos--;
	}
      stpos++;
      scrpos++;
    }

   /* Test ob WahlVorgabe legal */
   if (wahl>anz_punkte) wahl=anz_punkte;

   /* 1. mal Cursor schreiben */
   XBM_SetzteCursor(punkt_pos[wahl],punkt_string[wahl],para);

   /* Hauptschleife */
   while (taste!=13 && !(taste==27 && para->esc_erlaubt))
    {
      if (para->taste1==BM_NOKEY)
	{
	   while (!kbhit());
	   taste=(char)getch();
	}
       else {
	      taste=para->taste1;
	      para->taste1=para->taste2;
	      para->taste2=para->taste3;
	      para->taste3=BM_NOKEY;
	    }

      if (taste==0)
	 {
	   if (para->taste1==BM_NOKEY)
	     taste=(char)getch();
	     else {
		    taste=para->taste1;
		    para->taste1=para->taste2;
		    para->taste2=para->taste3;
		    para->taste3=BM_NOKEY;
		  }

	   switch (taste)
	    {
	      case 77:/* rechts */
		      if ((wahl+1)<anz_punkte)
			{
			  XBM_LoescheCursor(punkt_pos[wahl],
					  punkt_string[wahl],para);
			  wahl++;
			  XBM_SetzteCursor(punkt_pos[wahl],
					 punkt_string[wahl],para);
			}
		      break;
	      case 75:/* links */
		      if (wahl>0)
			 {
			  XBM_LoescheCursor(punkt_pos[wahl],
					  punkt_string[wahl],para);
			  wahl--;
			  XBM_SetzteCursor(punkt_pos[wahl],
					 punkt_string[wahl],para);
			 }
		      break;
	      case 72:/* rauf */
		      if (para->raus_rauf)
			taste=13;
		      break;
	      case 80:/* runter */
		      if (para->raus_runter)
			taste=13;
	    }
	 }
    }

   _setbkcolor(oldpap);
   _settextcolor(oldpen);

   /* Benutzervariable setzten */
   if (wahl>=0 && wahl<20)
    BM_letzte_wahl(wahl);

   if (taste==27) return BM_ESC;
   return wahl;
}

/*+++ Interne Subroutinen +++*/

/* Ausgabe eines Zeichens */
void XBM_outchar(char c)
{
  char s[2]=" ";

  s[0]=c;
  _outtext(s);
}

/* Cursor invers ausgeben */
void XBM_SetzteCursor(char pos,char *punkt_string,struct BM_parameter *p)
{
   long oldpap=_getbkcolor();
   short oldpen=_gettextcolor();

   _settextcolor(p->invers_pen);
   _setbkcolor(p->invers_paper);
   _settextposition(p->ypos,p->xpos+pos);
   _outtext(punkt_string);
   _settextcolor(oldpen);
   _setbkcolor(oldpap);
}

/* Cursor normal ausgeben */
void XBM_LoescheCursor(char pos,char *punkt_string,struct BM_parameter *p)
{
   _settextposition(p->ypos,p->xpos+pos);
   _outtext(punkt_string);
}
