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


#include "editor.h"
#include "cncsim.h"
#include "simul.h"
#include "diskrout.h"

/* interne Prototypen & definitionen */

#define rauf	 1
#define runter	 2
#define menue	 3
#define ctrl_y	 4
#define ctrl_ret 5
#define ctrl_ins 6
#define ctrl_pu  7
#define ctrl_pd  8
#define fun1	 9
#define fun12	 10

struct cncprgstr puffer;
struct LE_parameter linie_edit={NULL,NULL,0,12, 0,0, 1,1,0,0,0,1,1,1};

extern struct cncprgstr cncprg[max_line];  /* Prglisting */
struct cncprgstr *cncpgm;

extern char prg_gesichert;		   /* Save-Flag */
extern struct configstr prgconfig;	   /* Configuration */
extern char		prgerr[max_line];  /* Errorflag fuer jede Zeile */

extern char prgname[51],    /* PrgInfos */
	    autor[31],
	    sektion[31],
	    datum[21],
	    bemerk[51],
	    st_durchmesser[6],
	    st_laenge[6],
	    st_innendm[6],
	    savename[9];

void Maske(void);
void ZeileAusgeben(char,short);
void DatensatzInit(struct cncprgstr*,short);
char EditZeile(short);
char EditorMenue(void);
void Werkzeuge_darstellen(void);
void Neues_Programm(void);
void Programm_drucken(void);
char Print(char,char*);
void Help(void);
void CR(void);

void Zeile_Format(short);
char BuchstabenAnalyse(char*,char*);
char IN(char,char*);

char Pruefe_GMBefehl(char *in_gm, char xi,char zk,char flkt,char h); /*fuer die Zukunft vorgesehen*/
void Blende_Error_ein(short);

/* Funktionen */

void Editor(struct cncprgstr *cncpg)
{
   char raus=0,ret,xwahl;
   short zeile=0,za,zb;

   prg_gesichert=0;
   cncpgm=cncpg; /* "Globaler" Zeiger definieren */
   Maske();

   do {
	ret=EditZeile(zeile);
	switch(ret)
	  {
	    case runter:/* Scroll rauf */
			if (zeile<max_line-1)
			{
			  /* Oberes Fenster */
			  _settextwindow(2,1,9,80);
			  _scrolltextwindow(_GSCROLLUP);
			  ZeileAusgeben(8,zeile);
			  /* Unteres Fenster */
			  _settextwindow(14,1,24,80);
			  _scrolltextwindow(_GSCROLLUP);
			  _settextwindow(1,1,25,80);
			  ZeileAusgeben(24,zeile+12);

			  zeile++;
			  ZeileAusgeben(12,zeile);
			}
			break;
	    case rauf  :/* Scroll runter */
			if (zeile>0)
			{
			  /* Oberes Fenster */
			  _settextwindow(2,1,9,80);
			  _scrolltextwindow(_GSCROLLDOWN);
			  ZeileAusgeben(1,zeile-9);
			  /* Unteres Fenster */
			  _settextwindow(14,1,24,80);
			  _scrolltextwindow(_GSCROLLDOWN);
			  _settextwindow(1,1,25,80);
			  ZeileAusgeben(14,zeile);

			  zeile--;
			  ZeileAusgeben(12,zeile);
			}
			break;
	    case menue: {
			  /* Balkenmenue */
			  xwahl=EditorMenue();
			  if (xwahl==0) raus=1;

			  /* Alles neu Ausgeben */
			   for (za=1,zb=8;za<9;za++,zb--)
			     ZeileAusgeben((char)(za+1),zeile-zb);
			   ZeileAusgeben(12,zeile);
			   for (za=1;za<12;za++)
			     ZeileAusgeben((char)(za+13),zeile+za);
			}
			break;
	    case ctrl_y:if (zeile<max_line-1) /* Zeile loeschen */
			{
			  /* loeschen */
			  puffer=*(cncpgm+zeile);
			  for (za=zeile+1;za<max_line;za++)
			    *(cncpgm+za-1)=*(cncpgm+za);
			  DatensatzInit(cncpgm,max_line-1);

			  /* Sprungbefehle verbessern */
			  for (za=0;za<max_line;za++)
			    if ((cncpgm+za)->FLKT[0]=='L')
			      {
				zb=atoi((cncpgm+za)->FLKT+1);
				if (zb>zeile)
				  sprintf((cncpgm+za)->FLKT,"L%03d",zb-1);
			      }

			  /* Verschobenes Errorfeld loeschen */
			  ErrorInit();

			  /* Ausgeben */
			   for (za=1,zb=8;za<9;za++,zb--)
			     ZeileAusgeben((char)(za+1),zeile-zb);
			   ZeileAusgeben(12,zeile);
			   for (za=1;za<12;za++)
			     ZeileAusgeben((char)(za+13),zeile+za);

			}break;
	    case ctrl_ret: if (zeile<max_line-1) /* Zeile einfuegen */
		       {
			  /* Einfuegen */
			  puffer=*(cncpgm+max_line-1);
			  for (za=max_line-2;za>=zeile;za--)
			    *(cncpgm+za+1)=*(cncpgm+za);
			  DatensatzInit(cncpgm,zeile);

			  /* Sprungbefehle verbessern */
			  for (za=0;za<max_line;za++)
			    if ((cncpgm+za)->FLKT[0]=='L')
			      {
				zb=atoi((cncpgm+za)->FLKT+1);
				if (zb>zeile)
				  sprintf((cncpgm+za)->FLKT,"L%03d",zb+1);
			      }

			  /* Verschobenes Errorfeld loeschen */
			  ErrorInit();

			  /* Ausgabe */
			  for (za=1,zb=8;za<9;za++,zb--)
			     ZeileAusgeben((char)(za+1),zeile-zb);
			  ZeileAusgeben(12,zeile);
			  for (za=1;za<12;za++)
			     ZeileAusgeben((char)(za+13),zeile+za);

		       }break;
	    case ctrl_ins:if (zeile<max_line-1)
		       {
			  /* Puffer einfuegen */
			  for (za=max_line-2;za>=zeile;za--)
			    *(cncpgm+za+1)=*(cncpgm+za);
			  *(cncpgm+zeile)=puffer;

			  /* Sprungbefehle verbessern */
			  for (za=0;za<max_line;za++)
			    if ((cncpgm+za)->FLKT[0]=='L')
			      {
				zb=atoi((cncpgm+za)->FLKT+1);
				if (zb>zeile)
				  sprintf((cncpgm+za)->FLKT,"L%03d",zb+1);
			      }

			  /* Verschobenes Errorfeld loeschen */
			  ErrorInit();

			  /* Ausgeben */
			   for (za=1,zb=8;za<9;za++,zb--)
			     ZeileAusgeben((char)(za+1),zeile-zb);
			   ZeileAusgeben(12,zeile);
			   for (za=1;za<12;za++)
			     ZeileAusgeben((char)(za+13),zeile+za);

		       }break;
	    case ctrl_pu:{
			   /* Suche M30 */
			   for (za=0;za<max_line && strcmp((cncpgm+za)->GM,"M30 ");za++)
			    zeile=za;

			   /* Wenn kein M30 -> letzte Zeile */
			   if (za==max_line)
			    {
			      for (za=max_line-1;za>0 && !strcmp((cncpgm+za)->GM,"    ");za--)
				  zeile=za;
			    }

			   /* Ausgeben */
			   for (za=1,zb=8;za<9;za++,zb--)
			     ZeileAusgeben((char)(za+1),zeile-zb);
			   ZeileAusgeben(12,zeile);
			   for (za=1;za<12;za++)
			     ZeileAusgeben((char)(za+13),zeile+za);

			 }break;
	    case ctrl_pd:{
			   /* Oberes Fenster */
			   _settextwindow(2,1,9,80);
			   _clearscreen(_GWINDOW);
			   _settextwindow(1,1,25,80);
			   zeile=0;
			   ZeileAusgeben(12,0);
			   for (za=1;za<12;za++)
			     ZeileAusgeben((char)(za+13),za);
			 }break;
	    case fun1	:Blende_Error_ein(zeile); break;
	    case fun12	:CR();

	  } /* von switch zeileneditor-returnwert */

      } while(!raus);
}



char EditorMenue(void)
{
   struct BM_parameter edbm={NULL,11,1,1,11, 0,0,1, 0,1,1,BM_NOKEY,BM_NOKEY,BM_NOKEY};
   char balken[120]={"Editor: | Raus | | Programm | | Werkzeuge | | Drucken | | Neu | | Format/Syntax | | Hilfe |  "};
   char wahl,textmenge[100]={"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789)(&-=+*!.,:„”"};

   /* Bei Menuewahl: Fadenkreuz zeigen */
   SYS_PushColor();
   _settextcolor(0);
   _setbkcolor(4);
   TW_Open_Window(60,17,13,5,TW_RAHMEN_S1,TW_TITEL_OBEN,1,"Draufsicht");
   _settextposition(1,1);
   _outtext("     -X\n");
   _outtext("      ³ \n");
   _outtext(" -Z ÄÄÅÄÄ +Z\n");
   _outtext("      ³\n");
   _outtext("     +X");
   SYS_PopColor();

   _settextwindow(1,1,25,80);

   edbm.balken=balken;
   wahl=BM_Menue(&edbm);

   TW_Close_Window();

   SYS_PushColor();
   _settextcolor(1);
   _setbkcolor(11);
   _settextposition(1,1);
   _outtext("Editor:  Raus   Programm   Werkzeuge   Drucken   Neu   Format/Syntax   Hilfe  ");
   SYS_PopColor();

   if (wahl==0) return 0;
   switch (wahl)
   {
     case 1:Programmdaten_eingeben(); break;
     case 2:Werkzeuge_darstellen();   break;
     case 3:Programm_drucken();       break;
     case 4:Neues_Programm();	      break;
     case 5:{
	      Format();
	      Syntax();
	    } break;
     case 6:Help();
   }
   return 1;
}


char EditZeile(short zeile)
{
  char ret=0,
       xfeld=1,
       retLE,
       g_menge[15]={"01234567890M"},
       x_menge[15]={"-I01234567890"},
       z_menge[15]={"-K01234567890"},
       f_menge[15]={"LKT0123456789"},
       h_menge[15]={"0123456789"},
       bem_menge[80]={"1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.:/!=+*-&() "};

  /* Grundparameter einstellen */
  linie_edit.raus_unten=1;
  linie_edit.raus_oben=1;
  if (zeile==0) linie_edit.raus_oben=0;
  if (zeile==max_line-1) linie_edit.raus_unten=0;
  linie_edit.insert=0;

  SYS_CursorOn();
  /* Schleife Felder editieren */
  do {
       linie_edit.raus_rechts=1;
       linie_edit.raus_links=1;
       linie_edit.raus_ueberlauf=1;

       switch (xfeld)
       {
	 case 1:/* G/M-Feld */
		{
		  /* Parameter einstellen & GM editieren */
		  linie_edit.editstr=(cncpgm+zeile)->GM;
		  linie_edit.zeichenmenge=g_menge;
		  linie_edit.raus_links=0;
		  linie_edit.x=5;
		  retLE=LE_LineEdit(&linie_edit);

		  /* Returnwert auswerten */
		  switch(retLE)
		  {
		    case LE_OK	:/* ENTER */
		    case LE_RECHTS_RAUS:/* Ausgang nach rechts */
				 {
				   linie_edit.begin_xpos=0;
				   xfeld++;
				 }
				 break;
		    case LE_OBEN_RAUS:/* Ausgang nach oben */
				 ret=rauf;
				 break;
		    case LE_UNTEN_RAUS:/* Ausgang nach unten	*/
				 ret=runter; break;
		  }
		}
		break;
	 case 2:/* X/I-Feld */
		{
		  /* Parameter einstellen & XI editieren */
		  linie_edit.editstr=(cncpgm+zeile)->XI;
		  linie_edit.zeichenmenge=x_menge;
		  linie_edit.x=11;
		  retLE=LE_LineEdit(&linie_edit);

		  /* Returnwert auswerten */
		  switch(retLE)
		  {
		    case LE_OK	:/* ENTER */
		    case LE_RECHTS_RAUS:/* Ausgang nach rechts */
				 {
				   linie_edit.begin_xpos=0;
				   xfeld++;
				 }
				 break;
		    case LE_LINKS_RAUS:/* Ausgang nach links */
				 {
				   linie_edit.begin_xpos=3;
				   xfeld--;
				 }
				 break;
		    case LE_OBEN_RAUS:/* Ausgang nach oben */
				 ret=rauf;
				 break;
		    case LE_UNTEN_RAUS:/* Ausgang nach unten	*/
				 ret=runter; break;
		  }
		}
		break;
	 case 3:/* Z/K-Feld */
		{
		  /* Parameter einstellen & ZK editieren */
		  linie_edit.editstr=(cncpgm+zeile)->ZK;
		  linie_edit.zeichenmenge=z_menge;
		  linie_edit.x=20;
		  retLE=LE_LineEdit(&linie_edit);

		  /* Returnwert auswerten */
		  switch(retLE)
		  {
		    case LE_OK	:/* ENTER */
		    case LE_RECHTS_RAUS:/* Ausgang nach rechts */
				 {
				   linie_edit.begin_xpos=0;
				   xfeld++;
				 }
				 break;
		    case LE_LINKS_RAUS:/* Ausgang nach links */
				 {
				   linie_edit.begin_xpos=5;
				   xfeld--;
				 }
				 break;
		    case LE_OBEN_RAUS:/* Ausgang nach oben */
				 ret=rauf;
				 break;
		    case LE_UNTEN_RAUS:/* Ausgang nach unten	*/
				 ret=runter; break;
		  }
		}
		break;
	 case 4:/* F/L/K/T-Feld */
		{
		  /* Parameter einstellen & FLKT editieren */
		  linie_edit.editstr=(cncpgm+zeile)->FLKT;
		  linie_edit.zeichenmenge=f_menge;
		  linie_edit.x=29;
		  retLE=LE_LineEdit(&linie_edit);

		  /* Returnwert auswerten */
		  switch(retLE)
		  {
		    case LE_OK	:/* ENTER */
		    case LE_RECHTS_RAUS:/* Ausgang nach rechts */
				 {
				   linie_edit.begin_xpos=0;
				   xfeld++;
				 }
				 break;
		    case LE_LINKS_RAUS:/* Ausgang nach links */
				 {
				   linie_edit.begin_xpos=5;
				   xfeld--;
				 }
				 break;
		    case LE_OBEN_RAUS:/* Ausgang nach oben */
				 ret=rauf;
				 break;
		    case LE_UNTEN_RAUS:/* Ausgang nach unten	*/
				 ret=runter; break;
		  }
		}
		break;
	 case 5:/* H-Feld */
		{
		  /* Parameter einstellen & H editieren */
		  linie_edit.editstr=(cncpgm+zeile)->H;
		  linie_edit.zeichenmenge=h_menge;
		  linie_edit.x=36;
		  retLE=LE_LineEdit(&linie_edit);

		  /* Returnwert auswerten */
		  switch(retLE)
		  {
		    case LE_OK	:/* ENTER */
		    case LE_RECHTS_RAUS:/* Ausgang nach rechts */
				 {
				   linie_edit.begin_xpos=0;
				   xfeld++;
				 }
				 break;
		    case LE_LINKS_RAUS:/* Ausgang nach links */
				 {
				   linie_edit.begin_xpos=3;
				   xfeld--;
				 }
				 break;
		    case LE_OBEN_RAUS:/* Ausgang nach oben */
				 ret=rauf;
				 break;
		    case LE_UNTEN_RAUS:/* Ausgang nach unten	*/
				 ret=runter; break;
		  }
		}
		break;
	 case 6:/* Bemerkungs-Feld */
		{
		  /* Parameter einstellen & BEM editieren */
		  linie_edit.editstr=(cncpgm+zeile)->BEM;
		  linie_edit.zeichenmenge=bem_menge;
		  linie_edit.raus_rechts=0;
		  linie_edit.raus_ueberlauf=0;
		  linie_edit.x=41;
		  retLE=LE_LineEdit(&linie_edit);

		  /* Returnwert auswerten */
		  switch(retLE)
		  {
		    case LE_LINKS_RAUS:/* Ausgang nach links */
				 {
				   linie_edit.begin_xpos=3;
				   xfeld--;
				 }
				 break;
		    case LE_OBEN_RAUS:/* Ausgang nach oben */
				 ret=rauf;
				 break;
		    case LE_UNTEN_RAUS:/* Ausgang nach unten	*/
				 ret=runter; break;
		    case LE_OK	:/* ENTER */
			if (linie_edit.raus_unten)
			    ret=runter;
		  }
		}
       } /* von Hauptswitch */

       /* Generelle Auswertung */
       switch(retLE)
	{
	   case LE_ESC :/* Esc */
		       ret=menue; break;
	   case LE_CTRL_Y: ret=ctrl_y; break;
	   case LE_CTRL_RET: ret=ctrl_ret; break;
	   case LE_CTRL_INS: ret=ctrl_ins; break;
	   case LE_CTRL_PU: ret=ctrl_pd; break;
	   case LE_CTRL_PD: ret=ctrl_pu; break;
	   case LE_FUN_1  : ret=fun1; break;
	   case LE_FUN_12 : ret=fun12; break;
	}

     } while (!ret);

  SYS_CursorOff();
  return ret;
}

void ZeileAusgeben(char yp,short zeile)
{
   char buf[30];
   short pen=_gettextcolor();

   if (zeile<0 || zeile>=max_line)
    {
      _settextposition(yp,1);
      _outtext("                                        ");
      _outtext("                                        ");
    }
    else {
	   /* Nummer */
	   _settextposition(yp,1);
	   sprintf(buf,"%03d",zeile);

	   if (prgerr[zeile]) _settextcolor(10);  /* Error wenn nicht Null	*/
						  /* mit & 0xef: Ausser BIT 4 = Warnung	*/
	   _outtext(buf);
	   _settextcolor(pen);

	   /* G/M */
	   _settextposition(yp,5);
	   _outtext((cncpgm+zeile)->GM);
	   /* X/I */
	   _settextposition(yp,11);
	   _outtext((cncpgm+zeile)->XI);
	   /* Z/K */
	   _settextposition(yp,20);
	   _outtext((cncpgm+zeile)->ZK);
	   /* F/L/K/T */
	   _settextposition(yp,29);
	   _outtext((cncpgm+zeile)->FLKT);
	   /* H */
	   _settextposition(yp,36);
	   _outtext((cncpgm+zeile)->H);
	   /* BEM */
	   _settextposition(yp,41);
	   _outtext((cncpgm+zeile)->BEM);
	 }
}

void Maske(void)
{
   short x;
   _clearscreen(_GCLEARSCREEN);
   SYS_PushColor();
   _settextcolor(1);
   _setbkcolor(11);
   _settextposition(1,1);
   _outtext("Editor:  Raus   Programm   Werkzeuge   Drucken   Neu   Format/Syntax   Hilfe  ");
   SYS_PopColor();
   _settextposition(10,1);
   _outtext("ÄÄÄÂÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");
   _outtext(" N ³G/M ³  X/I   ³  Z/K   ³F/L/K/T³ H  ³ Bemerkung\n");
   _outtext("   ³    ³        ³        ³       ³    ³\n");
   _outtext("ÄÄÄÁÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");
   ZeileAusgeben(12,0);
   for (x=1;x<12;x++)
    ZeileAusgeben((char)(x+13),x);
   SYS_PushColor();
   _settextcolor(10);
   _setbkcolor(9);
   _outtext("\nBitte machen Sie Ihre Eingaben.                                                ");
   SYS_PopColor();
}


void DatensatzInit(struct cncprgstr *cncpg,short pos)
{
   strcpy((cncpg+pos)->GM,"    ");
   strcpy((cncpg+pos)->XI,"      ");
   strcpy((cncpg+pos)->ZK,"      ");
   strcpy((cncpg+pos)->FLKT,"    ");
   strcpy((cncpg+pos)->H,"    ");
   strcpy((cncpg+pos)->BEM,"                                       ");
}

void DatenfeldInit(struct cncprgstr *cncpg)
{
  short x;

  /* Prg loeschen */
  for (x=0;x<max_line;x++)
     DatensatzInit(cncpg,x);
  /* Fehler-Feld loeschen */
  ErrorInit();
  /* Prginfos loeschen */
  strcpy(prgname,"                                                  ");
  strcpy(autor	,"                              ");
  strcpy(sektion,"                              ");
  strcpy(datum	,"                    ");
  strcpy(bemerk ,"                                                  ");
  strcpy(st_durchmesser,"     ");
  strcpy(st_laenge     ,"     ");
  strcpy(st_innendm    ,"     ");
  strcpy(savename      ,"        ");
}



/*===============================================================
  MENUEROUTINEN
  ===============================================================*/

/****************************************************************
HELP
****************************************************************/
void Help()
{
   SYS_PushColor();
   _settextcolor(15);
   _setbkcolor(0);
   _setactivepage(1);

   /* Seite 1 */
   SYS_TextInvert();
   _settextposition(1,1);
   _outtext(" EDITOR - HILFSBILDSCHIRM : \n\n");
   SYS_TextInvert();
   _settextcolor(14);
   _outtext("Editorfunktionen:\n\n");
   _settextcolor(15);
   _outtext("CTRL + Y       : Aktuelle Zeile wird gel”scht und in den Puffer kopiert\n");
   _outtext("CTRL + INS     : Puffer auf der aktuellen Position einfgen\n");
   _outtext("CTRL + ENTER   : Leerzeile einfgen\n");
   _outtext("CTRL + PageDown: M30 oder letzte Zeile suchen\n");
   _outtext("CTRL + PageUp  : 1 Zeile \n");
   _outtext("ESC            : Men\n");
   _outtext("F1             : Fehlermeldung der aktuellen Zeile anzeigen\n");
   _outtext("INSERT         : Einfge-Modus umschalten\n\n");

   _settextcolor(14);
   _outtext("Bekannte Funktionen:\n\n");
   _settextcolor(15);

   _outtext(" G00  G01  G02  G03  G04  G21  G25  G26  G27  G33  G84  G86  G88  G90\n");
   _outtext(" G92  G92\n\n");
   _outtext(" M00  M03  M05  M06  M17  M30  M98  M99 \n");
   _outtext(" Pseudofunktion: M01: Stck neuzeichnen & Meldung ausgeben (fr Demoschleifen)\n");
   _outtext("                      ACHTUNG: Dieser Befehl erzeugt ein ERROR beim Syntaxcheck:\n\n");
   _outtext(" z.Z. noch nicht untersttzte Funktion: G73 G81 G82 G83 G85 G89 \n\n\n");

   _settextcolor(1);
   _outtext("[Taste]");
   _setvisualpage(1);
   getch();

   /* Seite 2 */
   _clearscreen(_GCLEARSCREEN);
   _settextcolor(15);
   SYS_TextInvert();
   _settextposition(1,1);
   _outtext(" EDITOR - HILFSBILDSCHIRM : \n\n");
   SYS_TextInvert();
   _settextcolor(14);
   _outtext("Menfunktionen:\n\n");
   _settextcolor(15);
   _outtext("Raus       : Zurck zum Hauptmen\n");
   _outtext("Programm   : ProgrammInfos eingeben/„ndern\n");
   _outtext("Werkzeuge  : Werkzeugbildschirm einblenden\n");
   _outtext("Drucken    : Aktuelles CNC-Programm drucken\n");
   _outtext("Neu        : Aktuelles Programm l”schen\n");
   _outtext("Format/Syntax: - Programm ins EMCO-Format bringen\n");
   _outtext("               - Programm berprfen:\n");
   _outtext("                   M/G-Befehle\n");
   _outtext("                   Parameteranzahl\n");
   _outtext("                   I/K/L/T berprfen\n");
   _outtext("     Sinn dieser  šberprfung  ist  es,  der  EMCO-Maschine  keine  falschen\n");
   _outtext("     Programminformationen zu bertragen, und nicht die Funktionstchtigkeit\n");
   _outtext("     eines Programms zu  testen.  Ausserdem  ist  diese  šberprfung relativ\n");
   _outtext("     oberfl„chlich. Der Programmierer sollte also immer das Programm vor der\n");
   _outtext("     šbertragen nochmals durchgehen, um ein Chaos auf der  EMCO-Maschine  zu\n");
   _outtext("     vermeiden.\n");
   _outtext("     HINWEIS: Fehlende H-Werte werden durch den Standartwert ersetzt, der im\n");
   _outtext("              Optionsmen eingestellt werden kann.Trotzdem wird eine Fehler-\n");
   _outtext("              meldung erzeugt.\n\n");

   _settextcolor(1);
   _outtext("[Taste fhrt zum Editor]");
   _setvisualpage(1);
   getch();

   /* Seite 3 */
   _clearscreen(_GCLEARSCREEN);
   _settextcolor(15);
   SYS_TextInvert();
   _settextposition(1,1);
   _outtext(" EDITOR - HILFSBILDSCHIRM : \n\n");
   SYS_TextInvert();
   _settextcolor(14);
   _outtext("Grafikausgabe:\n\n");
   _settextcolor(15);

   _outtext("Bei der VGA-Version wird im  Simulator  das  ganze Stck dargestellt.\n");
   _outtext("Im Gegensatz hirzu wird in der  CGA-Version  nur  die  untere  H„lfte\n");
   _outtext("gezeichnet.\n");
   _outtext("Da die CGA-Karte nur eine geringe Grafikaufl”sung  untersttzt,  wird\n");
   _outtext("es in der Simulation zwangsl„ufig zu Ungenauigkeiten kommen, die aber\n");
   _outtext("keine Auswirkungen auf das reelle Drehstck haben.\n\n\n\n\n\n");



   /* Raus aus Hilfe */
   _settextcolor(1);
   _outtext("[Taste fhrt zum Editor]");
   _setvisualpage(1);
   getch();

   _setvisualpage(0);
   _setactivepage(0);
   SYS_PopColor();
}

/****************************************************************
Programm-Format 
*****************************************************************/
void Format(void)
{
   short bis_zeile=max_line-1,
	 zeile,esc=0;
   char buf[80];

   /* letzte Zeile suchen */
   for (zeile=max_line-1;zeile>0 && !strcmp(cncprg[zeile].GM,"    ");zeile--)
     bis_zeile=zeile;

   /* Raus wenn kein Programm im Speicher */
   if (bis_zeile==1) return;

   /* Fenster oeffnen & maske */
   SYS_PushColor();
   _settextcolor(7);
   _setbkcolor(1);
   TW_Open_Window(15,10,31,3,TW_RAHMEN_D2,TW_TITEL_OBEN,3,"Programm formatieren");

   _outtext("\n Formatiere Zeile     von ");
   sprintf(buf,"%03d",bis_zeile-1);
   _outtext(buf);

   for (zeile=0;zeile<bis_zeile && !esc;zeile++)
    {
      sprintf(buf,"%03d",zeile);
      _settextposition(2,19);
      _outtext(buf);
      Zeile_Format(zeile);
      if (kbhit()) esc=(getch()==27) ? 1:0;
    }

   TW_Close_Window();
   SYS_PopColor();
}

void Zeile_Format(short zeile)
{
   char zwbuf[20],endbuf[20];

   /* GM */
   if (BuchstabenAnalyse(cncprg[zeile].GM,endbuf)) /* keine Aktion wenn Feld leer (return 0)*/
    {
      sprintf(zwbuf,"%02d ",atoi(cncprg[zeile].GM));
      strcat(endbuf,zwbuf);
      strcpy(cncprg[zeile].GM,endbuf);
    }

   /* XI */
   if(BuchstabenAnalyse(cncprg[zeile].XI,endbuf))
     {
       sprintf(zwbuf,"%4d ",atoi(cncprg[zeile].XI));
       if (atoi(cncprg[zeile].XI)<10) zwbuf[2]='0';
       strcat(endbuf,zwbuf);
       strcpy(cncprg[zeile].XI,endbuf);
     }

   /* ZK */
   if (BuchstabenAnalyse(cncprg[zeile].ZK,endbuf))
     {
       sprintf(zwbuf,"%5d",atoi(cncprg[zeile].ZK));
       if (atoi(cncprg[zeile].ZK)<10) zwbuf[3]='0';
       strcat(endbuf,zwbuf);
       strcpy(cncprg[zeile].ZK,endbuf);
     }

   /* FLKT */
   if (BuchstabenAnalyse(cncprg[zeile].FLKT,endbuf))
     {
       sprintf(zwbuf,"%03d",atoi(cncprg[zeile].FLKT));
       if (atoi(cncprg[zeile].FLKT)<100) zwbuf[0]=' ';
       strcat(endbuf,zwbuf);
       strcpy(cncprg[zeile].FLKT,endbuf);
     }

   /* H */
   if (strcmp(cncprg[zeile].H,"    "))
     {
	sprintf(cncprg[zeile].H,"%03d ",atoi(cncprg[zeile].H));
	if (atoi(cncprg[zeile].H)<100) cncprg[zeile].H[0]=' ';
     }
}

char BuchstabenAnalyse(char *str,char *ziel)
{
   short za;

   /* Wenn ein Buchstabe oder ein Minus vorhanden, dann auf erste Position,
      sonst Leerzeichen */
   char *menge={"-MLKTIK"}, leer[30];

   strcpy(leer,"               ");
   leer[strlen(str)]=0;
   if (!strcmp(leer,str))
     return 0;
     else { /* Nur wenn Feld nicht leer */
	    strcpy(ziel," "); /* Wenn kein Zeichen */
	    for(za=0;str[za]!=0;za++)
	    if (IN(str[za],menge))
	       {
		 *ziel=str[za];
		 str[za]=' ';
	       }
	  }
   return 1;
}


char IN(char c,char *menge)
{
  char z,ret=0;

  for (z=0;menge[z]!=0 && !ret;z++)
     if (menge[z]==c) ret=1;

  return ret;
}

/****************************************************************
DRUCKEN
*****************************************************************/
void Programm_drucken(void)
{
   short bis_zeile,
	 zeile;
   char retcode=0;
   char dr_status,buf[10];

   /* letzte Zeile suchen */
   for (zeile=max_line-1;zeile>0 && !strcmp(cncprg[zeile].GM,"    ");zeile--)
     bis_zeile=zeile;

   /* Raus wenn kein Programm im Speicher */
   if (bis_zeile==1) return;

   /* Fenster oeffnen & maske */
   SYS_PushColor();
   _settextcolor(7);
   _setbkcolor(1);
   TW_Open_Window(15,10,50,10,TW_RAHMEN_D2,TW_TITEL_OBEN,3,"Programmlisting drucken");

   /* Init Drucker & Test ob LPT vorhanden */
   if (SYS_DruckerInit(prgconfig.lpt) & 1)
    {
      _outtext(" \aDrucker an Konfigurierter LPT nicht vorhanden!\n");
      _outtext("      Bitte „ndern Sie dir Konfiguration.");
      getch();
    }
   else
    {
      _settextposition(2,1);
      _outtext(".Bitte Papier einlegen & Drucker ON-LINE schalten.");
      _settextposition(2,1);

      /* Warten bis Drucker ON-LINE */
      do {
	   dr_status=SYS_DruckerStatus(prgconfig.lpt);
	   if (kbhit()) retcode=(char)getch();
	 } while (!(dr_status & 16) && retcode!=27);

      /* Drucken */
      if (retcode!=27) /* 27=ErrorReturnCode von Print() */
	{
	  _outtext(" Drucke Programminfos...\n");

	  /* Drucker einstellen: unidirektional */
	  buf[0]=27; buf[1]='U'; buf[2]=1; buf[3]=0;
	  retcode=Print(0,buf);

	  /* Infos */
	  retcode=Print(retcode,"\n\n Programm : ");
	  retcode=Print(retcode,prgname);
	  retcode=Print(retcode,"\n Autor    : ");
	  retcode=Print(retcode,autor);
	  retcode=Print(retcode,"\n Datum    : ");
	  retcode=Print(retcode,datum);
	  retcode=Print(retcode,"\n Komment. : ");
	  retcode=Print(retcode,bemerk);
	  retcode=Print(retcode,"\n Werkstck: Auáendurchmesser ");
	  retcode=Print(retcode,st_durchmesser);
	  retcode=Print(retcode," 1/100mm\n            Innendurchmesser ");
	  retcode=Print(retcode,st_innendm);
	  retcode=Print(retcode," 1/100mm\n            L„nge            ");
	  retcode=Print(retcode,st_laenge);
	  retcode=Print(retcode," 1/100mm");

	  /* Kopf */
	  retcode=Print(retcode,"\n\n ÄÄÄÂÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");
	  retcode=Print(retcode,"  N ³G/M ³  X/I   ³  Z/K   ³F/L/K/T³ H  ³ Bemerkung\n");
	  retcode=Print(retcode," ÄÄÄÅÄÄÄÄÅÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");

	  _outtext(" Drucke Zeile       von ");
	  sprintf(buf,"%03d",bis_zeile);
	  _outtext(buf);
	  _outtext("                       ");

	  /* Programm */
	  for (zeile=0;zeile<bis_zeile && retcode!=27;zeile++)
	    {
	      sprintf(buf,"%03d",zeile);

	      _settextposition(3,16);
	      _outtext(buf);
	      retcode=Print(retcode," ");
	      retcode=Print(retcode,buf);
	      retcode=Print(retcode,"³");

	      /* G/M */
	      retcode=Print(retcode,(cncpgm+zeile)->GM);
	      retcode=Print(retcode,"³ ");
	      /* X/I */
	      retcode=Print(retcode,(cncpgm+zeile)->XI);
	      retcode=Print(retcode," ³ ");
	      /* Z/K */
	      retcode=Print(retcode,(cncpgm+zeile)->ZK);
	      retcode=Print(retcode," ³ ");
	      /* F/L/K/T */
	      retcode=Print(retcode,(cncpgm+zeile)->FLKT);
	      retcode=Print(retcode,"  ³");
	      /* H */
	      retcode=Print(retcode,(cncpgm+zeile)->H);
	      retcode=Print(retcode,"³");
	      /* BEM */
	      retcode=Print(retcode,(cncpgm+zeile)->BEM);
	      retcode=Print(retcode,"\n");
	    }
	    retcode=Print(retcode," ÄÄÄÁÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n\n\n");

	} /* von drucken wenn ON-LINE & not ESC */
    } /* von drucken */

   if (retcode!=27) {
		      _settextposition(9,2);
		      _outtext("Drucken beendet - Taste drcken.");
		      getch();
		    }

   TW_Close_Window();
   SYS_PopColor();
}

char Print(char erlaubt,char *string)
/* Ist Eingangsparameter=27=Error, dann no Aktion & raus mit Errorcode */
{
  #define error 27
  char status,za,ret=erlaubt,raus;
  short taste=0;

  for(za=0;string[za]!=0 && ret!=error;za++) /* Alle Zeichen drucken */

      do { /* Gleiches Zeichen bis Error oder ESC */

	   raus=1; /* Zeichen erfolgreich gedruckt */
	   status=SYS_DruckerPrint(prgconfig.lpt,string[za]); /* Drucken */

	   /* Test des Status & ESC */
	   if (kbhit()) taste=getch();
	   if (taste==27) ret=error;
	   if (status & 32) /* Out of Paper */
	     {
	       _settextposition(9,1);
	       _outtext("*** Bitte neues Papier einlegen ***");
	       raus=0; /* Zeichen neu drucken */

	       taste=0;
	       while (taste!=27 && (SYS_DruckerStatus(prgconfig.lpt) & 32))
		     if (kbhit()) taste=getch();
	       if (taste==27) {
				raus=1;	   /* Zeichenschleife verlassen da ESC */
				ret=error; /* Returncode = Error */
			      }

	       _settextposition(9,1);
	       _outtext("                                   ");
	     } /* von OUT OF PAPER */

	   if (!(status & 16) || (status & 8)) /* Error */
	     {
	       raus=0; /* Zeichen neu drucken */

	       _settextposition(9,1);
	       _outtext("ACHTUNG! Es kann nicht weitergedruckt werden!\n");
	       _outtext("Weiter drucken?");
	       if (SYS_JaNein(18,10,SYS_JA,SYS_NEIN)==SYS_NEIN)
		{
		  raus=1;	  /* Zeichenschleife verlassen */
		  ret=error;	  /* Returncode = Error */
		}
	       _settextposition(9,1);
	       _outtext("                                             \n");
	       _outtext("                                              ");
	     }
	 } while (!raus); /* von Zeichenschleife */

  return ret;
}

/****************************************************************
PROGRAMMDATEN
*****************************************************************/
void Programmdaten_eingeben(void)
{
   char buf[30],
	feld=0,
	ret,
	textmenge[100]={"abcdefgh ijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789)(&-=+*!.,:„”"};
   struct dosdate_t aktdat;
   struct LE_parameter edpg={NULL,NULL,15,0,0,0,0,0,0,0,0,1,0,1};
   /* externe Var */
   extern char prgname[51],
	       autor[31],
	       sektion[31],
	       datum[21],
	       bemerk[51],
	       st_durchmesser[6],
	       st_innendm[6],
	       st_laenge[6];

   SYS_PushColor();
   _settextcolor(1);
   _setbkcolor(11);
   TW_Open_Window(7,7,66,13, TW_RAHMEN_D2,TW_TITEL_UNTEN,5,"Programminfos:");

   /* wenn DATUM leer, dann aktuelles nehmen */
   if (!strcmp(datum,"                    "))
     {
      _dos_getdate(&aktdat);
      sprintf(buf,"%02u/%02u/%4u          ",aktdat.day,aktdat.month,aktdat.year);
      strcpy(datum,buf);
     }

   /* erste Ausgabe */
   _outtext("Bitte geben Sie die Programminformationen ein (Beenden ESC):\n");
   _outtext("\nProgrammtitel:");
   _outtext(prgname);
   _outtext("\nAutor        :");
   _outtext(autor);
   _outtext("\nSektion      :");
   _outtext(sektion);
   _outtext("\nDatum        :");
   _outtext(datum);
   _outtext("\nBemerkung    :");
   _outtext(bemerk);
   _outtext("\n\nWerkstck: Einheit 1/100mm\n");
   _outtext("Durchmesser  :");
   _outtext(st_durchmesser);
   _outtext("\nL„nge        :");
   _outtext(st_laenge);
   _outtext("\nInnendmesser :");
   _outtext(st_innendm);

   /*Editieren*/
   SYS_CursorOn();
   do
    {
      edpg.zeichenmenge=textmenge;
      switch(feld)
	{
	  case 0:/*Titel*/
	    {
	      edpg.editstr=prgname;
	      edpg.raus_oben=0;
	      edpg.raus_unten=1;
	      edpg.y=3;
	      ret=LE_LineEdit(&edpg);
	      if (ret==LE_UNTEN_RAUS || ret==LE_OK) feld++;
	    }
	    break;
	  case 1:/*Autor*/
	    {
	      edpg.editstr=autor;
	      edpg.raus_oben=1;
	      edpg.raus_unten=1;
	      edpg.y=4;
	      ret=LE_LineEdit(&edpg);
	      if (ret==LE_UNTEN_RAUS || ret==LE_OK) feld++;
	      if (ret==LE_OBEN_RAUS) feld--;
	    }
	    break;
	  case 2:/*Sektion*/
	    {
	      edpg.editstr=sektion;
	      edpg.raus_oben=1;
	      edpg.raus_unten=1;
	      edpg.y=5;
	      ret=LE_LineEdit(&edpg);
	      if (ret==LE_UNTEN_RAUS || ret==LE_OK) feld++;
	      if (ret==LE_OBEN_RAUS) feld--;
	    }
	    break;
	  case 3:/*datum*/
	    {
	      edpg.editstr=datum;
	      edpg.raus_oben=1;
	      edpg.raus_unten=1;
	      edpg.y=6;
	      ret=LE_LineEdit(&edpg);
	      if (ret==LE_UNTEN_RAUS || ret==LE_OK) feld++;
	      if (ret==LE_OBEN_RAUS) feld--;
	    }
	    break;
	  case 4:/*bem*/
	    {
	      edpg.editstr=bemerk;
	      edpg.raus_oben=1;
	      edpg.raus_unten=1;
	      edpg.y=7;
	      ret=LE_LineEdit(&edpg);
	      if (ret==LE_OBEN_RAUS) feld--;
	      if (ret==LE_UNTEN_RAUS || ret==LE_OK) feld++;
	    }
	    break;
	  case 5:/* Durchmesser */
	    {
	      edpg.zeichenmenge="01234567890";
	      edpg.editstr=st_durchmesser;
	      edpg.raus_oben=1;
	      edpg.raus_unten=1;
	      edpg.y=10;
	      ret=LE_LineEdit(&edpg);
	      if (ret==LE_OBEN_RAUS) feld--;
	      if (ret==LE_UNTEN_RAUS || ret==LE_OK) feld++;
	    }
	    break;
	  case 6:/* Laenge */
	    {
	      edpg.zeichenmenge="0123456789";
	      edpg.editstr=st_laenge;
	      edpg.raus_oben=1;
	      edpg.raus_unten=1;
	      edpg.y=11;
	      ret=LE_LineEdit(&edpg);
	      if (ret==LE_OBEN_RAUS) feld--;
	      if (ret==LE_UNTEN_RAUS || ret==LE_OK) feld++;
	    } break;
	  case 7:/* Innendm */
	    {
	      edpg.zeichenmenge="0123456789";
	      edpg.editstr=st_innendm;
	      edpg.raus_oben=1;
	      edpg.raus_unten=0;
	      edpg.y=12;
	      ret=LE_LineEdit(&edpg);
	      if (ret==LE_OBEN_RAUS) feld--;
	    }
	} /* von Switch */
    } while(ret!=LE_ESC);

  SYS_CursorOff();
  TW_Close_Window();
  SYS_PopColor();
}

void Neues_Programm(void)
{
  char ret;

   SYS_PushColor();
  _settextcolor(7);
  _setbkcolor(6);
  TW_Open_Window(28,10,25,3,TW_RAHMEN_D1,TW_TITEL_UNTEN,3,"ACHTUNG");

  _outtext("    *** NACHFRAGE ***\n");
  _outtext("Neues Programm anmelden ?");
  ret=SYS_JaNein(7,3,0,0);
  TW_Close_Window();
  SYS_PopColor();

  if (ret)
    {
      prg_gesichert=1;
      ErrorInit();
      DatenfeldInit(cncpgm);
      Maske();
    }
}

/* Errorfeld initialisieren */
void ErrorInit(void)
{
  short x;			/* oder mit 'memset' */
  for (x=0;x<max_line;x++)
    prgerr[x]=0;
}

void Blende_Error_ein(short zeile)
{
   static char *meldung[]=
		     {"* Unbekannter G-/M-Befehl!\n",
		      "* Zuviele Parameter gefunden!\n",
		      "* Zuwenig Parameter gefunden!\n",
		      "* I/K in X/Z-Feld erwartet!\n",
		      "* H-Standartwert wurde eingefgt!\n",
		      "* L/K/T-Fehler im H-Feld!\n",
		      "* I/K im X/Z-Feld nicht erlaubt!\n",
		      "* H-Befehl nicht erlaubt!\n"};
   char bit;

   SYS_PushColor();
   _settextcolor(1);
   _setbkcolor(11);
   TW_Open_Window(42,10,35,10, TW_RAHMEN_D2,TW_TITEL_UNTEN,5,"Fehler:");
   _outtext("Beim der letzten Syntaxberprfung\naufgetretene Fehler:\n");

   /* Bits 0 bis 7 testen */
   for(bit=0;bit<8;bit++)
      if ( (prgerr[zeile]>>bit) & 1 ) _outtext(meldung[bit]);

   if (!prgerr[zeile])_outtext("\n\nKein Fehler in der aktuellen Zeile.");

   getch();
   SYS_PopColor();
   TW_Close_Window();
}

void CR(void)
{
   char copyright[255]="---";

   unsigned char za=0;
   char buf[10];

   SYS_PushColor();
   _settextcolor(7);
   _setbkcolor(1);
   TW_Open_Window(15,10,39,7,TW_RAHMEN_D2,TW_TITEL_OBEN,3,"Copyright");

   for (za=0;za<238;za++)
    {
      buf[0]=(char)(copyright[za]-13);
      buf[1]=0;
      _outtext(buf);
    }

   getch();
   TW_Close_Window();
   SYS_PopColor();
}
