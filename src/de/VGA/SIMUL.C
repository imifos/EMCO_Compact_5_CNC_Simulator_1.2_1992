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

/* CNCSIM : Modul Simulator Version VGA */

#include <math.h>
#include <ctype.h>
#include <dos.h>
#include <time.h>
#include <sys\timeb.h>
#include "..\cncsim.h"
#include "..\simul.h"

extern struct configstr prgconfig;
extern struct cncprgstr cncprg[max_line];
extern char st_durchmesser[6];
extern char st_laenge[6];
extern char st_innendm[6];

/* Interne Definitionen */

/* Schrittweiten */
#define schritt_g00 130	  /* Eilgang */
#define schritt_g01 30	  /* Abnehmen */
#define schritt_g02 1	  /* Winkelschritte bei Kreisbewegungen */

/* HilfsWerte */
#define nicht	    0
#define escape	    1
#define noesc	    0
#define checkTasten 1
#define null	    1000

/* Constanten */
#define deg 0.0174533F /* Faktor DEG / RAD */

short ip,sp;
short stack[6];

short xpt,ypt; /* aktuelle Graphickoord */
char  old_tool;
char  tool;
short xcorr,zcorr; /* Korrekturwerte der Staehle, muessen in Absprg berechnet werden */

char meldung_str[80],
     status_str[30];

/* flags */
char inkrementell,
     trace,
     menue;

short xrad,    /* Radiuspos des Stahls */
      zpos;    /* Position des Stahl relativ zum StueckNP */

short grNPx,
      grNPy;

/* constanten */
short	 durchm,
	 radius, /* =durchm/2*/
	 laenge,
	 idurchm;

/* externe Var */
extern char st_durchmesser[6];
extern char st_laenge[6];

/* interne Funktionen */
short Unit2Pnt(short);
void Setzte_Stahl(short,short,char);
void Init_Simulator(void);
void Zeichne_stueck(void);
char Hole_DMuLNG(void);
void SimBildschirmmaske(void);
char Manuell_Nullpunkt_festlegen(void);
void Automatisch_Ankratzen(void);
void Status(char*);
void Meldung(char*);
void Wait(short);
void Koordinaten(short,short);
void DemoText(void);
double arc_wi(double,double,char*);
void ElimSpaces(char*);

void ZeileAusgabe(short);
void SUBZeileAusgabe(short,char);
char Check_FlagTasten(void);
char Execute_G(char*,char*);
char Execute_M(char*,char*);
char SucheM30(void);
void NeuerGrNP(short,short);

char Ziehe_Gerade(short,short,short,short,short,short,char);
char Ziehe_Kreis03(short,short,short,short,short);
char Ziehe_Kreis02(short,short,short,short,short);
char Ziehe_KreisNO90(short,short,short,short,short,short,char);
char Gewinde(short Xrad,short Zpos1,short Zpos2,short steigung,short warte);

/* Funktionen */

void CNCSimulation(void)
{
   char esc,ok,prg_ende=0;
   char t[20]={"Test Fall   Test e"};

   if (!Hole_DMuLNG())
    {
       /* Simulator initialisieren */
       Init_Simulator();
       SimBildschirmmaske();
       Init2SCR();

       /* Benutzer einstellen lassen */
       _setvieworg(0,240);		  /* Zum Ankratzen NP=MaschinenNP */
       Automatisch_Ankratzen(); 	  /* Nehmen wir dem Jungen die Arbeit ab.*/

       Setzte_Stahl(0,0,0);		  /* Um Fehler zu vermeiden - Stahl loeschen */

       /* Stuecknullpunkt fuer Einstellungen (neuer NP im prg)*/
       _setvieworg(Unit2Pnt(laenge),240);
       grNPx=Unit2Pnt(laenge);
       grNPy=240;

       ZeileAusgabe(0); /* Anzeige der ersten Zeile zur Orientierung */
       esc=Manuell_Nullpunkt_festlegen(); /* Stahl auf eigenen Nullpunkt steuern */

       if (!esc)
	   ok=SucheM30();

       if (!trace) {
		     Meldung("Taste zum Programmstart");
		     Status("Warte");
		     while (kbhit());
		     while (!kbhit());
		     prg_ende=Check_FlagTasten();
		   }

       /* Programm-Hauptscheife */
       if (!esc && ok)
	  for (ip=0;!prg_ende;ip++)
	    if (ip==max_line)
	      {
		Meldung("Zeilenlimit erreicht - M30 Åbersprungen");
		Status("Taste!");
		prg_ende=1;
	      } else
	      {
		ZeileAusgabe(ip);
		Zeile2SCR(ip);
		if (trace) {
			     Meldung("Warte auf Tastendruck...");
			     Status("Trace-Stop");
			     while (kbhit()) getch();
			     while (!kbhit());
			     prg_ende=Check_FlagTasten();
			   }
		if (!prg_ende)
		 {
		   if (cncprg[ip].GM[0]=='M') prg_ende=Execute_M( &ok,&esc);
		     else prg_ende=Execute_G(&ok,&esc);
		 }
	      }

       /* test ob noch UP auf Stack */
       if (sp!=0 && (!esc || !ok))
	{
	  Meldung("M30 im Unterprogramm gefunden!");
	  Status("Error!");
	}

       /* Simulation beendet */
       if (esc)
	{
	  Meldung("ESC-Taste betÑtigt - Taste zum Beenden.");
	  Status("Abbruch durch ESC!");
	}
       getch(); /* Taste zum ClrScr */

       _setvideomode(_TEXTC80);
       SYS_CursorOff();
       Bildschirmmaske();
    }
}

/* Simulationsfunctionen */

char Execute_M(char *ok,char *esc)
{
  char endprg=0, arghhhh=*esc,
       Mbuffer[6],buf[20];
  int  m_wert;
  int  xi_wert,
       zk_wert,
       flkt_wert,
       h_wert;

  Status("Simulation");
  Meldung("Bearbeite M-Befehl.");

  /* G/M umkopieren zwecks Veraenderung */
  strcpy(Mbuffer,cncprg[ip].GM);

  /* M-Zeichen eliminieren & Wert feststellen */
  Mbuffer[0]=' ';
  m_wert=atoi(Mbuffer);

  strcpy(buf,cncprg[ip].XI);
  ElimSpaces(buf);
  ElimSpaces(buf);
  if (isdigit((int)buf[0]) || buf[0]=='-')
     xi_wert=atoi(buf);
    else xi_wert=atoi(buf+1);

  strcpy(buf,cncprg[ip].ZK);
  ElimSpaces(buf);
  ElimSpaces(buf);
  if (isdigit((int)buf[0]) || buf[0]=='-')
     zk_wert=atoi(buf);
    else zk_wert=atoi(buf+1);

  strcpy(buf,cncprg[ip].FLKT);
  ElimSpaces(buf);
  ElimSpaces(buf);
  if (isdigit((int)buf[0]) || buf[0]=='-')
     flkt_wert=atoi(buf);
    else flkt_wert=atoi(buf+1);

  h_wert=atoi(cncprg[ip].H);

  /* M-Befehle verzweigen */
  switch(m_wert)
  {
     /**************** M00 *******************/
     case 0:{
	      Meldung("Taste zur Fortsetzung.");
	      getch();
	      Meldung(" ");
	    } break; /* No Action */
     /**************** M01: Pseudo: Stueck neuzeichnen *******************/
     case 1:{
	      DemoText();
	      Zeichne_stueck();
	    }break;
     /**************** M03 *******************/
     case 3:break; /* No Action */
     /**************** M05 *******************/
     case 5:break; /* No Action */
     /**************** M06 *******************/
     case 6: /* werkzeugwechsel */
	     if (cncprg[ip].FLKT[0]=='T' && flkt_wert>0 && flkt_wert<=max_tools)
	     {
	       /* Anzeige */
	       _settextposition(3,37);
	       _outtext(cncprg[ip].FLKT);
	       _outtext("        ");
	       Tool2SCR(cncprg[ip].FLKT);


	       Setzte_Stahl(xrad,zpos,(char)flkt_wert);
	       endprg=Ziehe_Gerade(xrad,zpos,xrad+xi_wert,zpos+zk_wert,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
	       xrad=xrad+xi_wert;
	       zpos=zpos+zk_wert;

	       xcorr=xi_wert;
	       zcorr=zk_wert;
	     } else { *ok=0; endprg=1; }
	     break;
     /**************** M17 *******************/
     case 17:if (sp>0)
	     {
	       ip=stack[--sp];
	       ip++;
	     } else
	     {
	       Meldung("Fehler im RETURN-M17-Befehl: Keine Subroutine!");
	       Status("Fehler!");
	       endprg=1; *ok=0;
	     }break;
     /**************** M30 *******************/
     case 30:{
	       Status("M30-Ende");
	       Meldung(" ");
	       endprg=1;
	     } break;
     /**************** M98 *******************/
     case 98:break; /* No Action */
     /**************** M99 *******************/
     case 99:{
	       /* nur in Verbindung mit G02/G03 */
	       Meldung("Einzelner M99 Befehl gefunden!!");
	       Status("Fehler!");
	       endprg=1;
	       *ok=0;
	     }
     /**************** Error *******************/
     default:/* Unbekannter M-Befehl */
	     {
	       *ok=0;	 /* Fehlerflag */
	       endprg=1; /* Prg-Ende-Flag */
	     }
  }/* von switch m_wert */

  /* Returnwert */
  if (!*ok && endprg)
	    {
	      Status("E-Unterbrechung");
	      Meldung("M-Befehl in der aktuellen Zeile ist unbekannt oder unkorrekt!");
	    }

  return endprg;
}

char Execute_G(char *ok,char *esc)
{
  char endprg=0,
       zw_buf1,buf[20];

  int  g_wert,
       xi_wert,
       zk_wert,
       flkt_wert,
       h_wert,
       taste;

  short P1x,P1z,P2x,P2z, steigung,
	step,Xpos,Zpos,za;
  struct dostime_t time;


  Status("Simulation");
  Meldung("Bearbeite G-Befehl.");

  /* eventuell test ob nicht G00 -> dann darf g_wert nicht 0 sein -> Err */

  /* Werte feststellen */

  g_wert=atoi(cncprg[ip].GM);

  strcpy(buf,cncprg[ip].XI);
  ElimSpaces(buf);
  ElimSpaces(buf);
  if (isdigit((int)buf[0]) || buf[0]=='-')
     xi_wert=atoi(buf);
    else xi_wert=atoi(buf+1);

  strcpy(buf,cncprg[ip].ZK);
  ElimSpaces(buf);
  ElimSpaces(buf);
  if (isdigit((int)buf[0]) || buf[0]=='-')
     zk_wert=atoi(buf);
    else zk_wert=atoi(buf+1);

  strcpy(buf,cncprg[ip].FLKT);
  ElimSpaces(buf);
  ElimSpaces(buf);
  if (isdigit((int)buf[0]) || buf[0]=='-')
     flkt_wert=atoi(buf);
    else flkt_wert=atoi(buf+1);

  h_wert=atoi(cncprg[ip].H);

  /* G-Befehle verzweigen */
  switch(g_wert)
  {
     /**************** G00 *******************/
     case 00:if (cncprg[ip].XI[0]!='I' && cncprg[ip].ZK[0]!='K')
	      {
		if (inkrementell)
		   { /* G00 INCR */
		     endprg=Ziehe_Gerade(xrad,zpos,xrad+xi_wert,zpos+zk_wert,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
		     if (endprg) { *ok=1; *esc=1; }
		     xrad=xrad+xi_wert;
		     zpos=zpos+zk_wert;
		   } else
		   { /* G00 ABS */
		     endprg=Ziehe_Gerade(xrad,zpos,(short)(xi_wert/2)+xcorr,zk_wert+zcorr,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
		     if (endprg) { *ok=1; *esc=1; }
		     xrad=(short)(xi_wert/2)+xcorr;
		     zpos=zk_wert+zcorr;
		   }
	      }
	       else { *ok=0; endprg=1; }
	     break;
     /**************** G01 *******************/
     case 01:if (cncprg[ip].XI[0]!='I' && cncprg[ip].ZK[0]!='K')
	      {
		if (inkrementell)
		   { /* G01 INCR */
		     endprg=Ziehe_Gerade(xrad,zpos,xrad+xi_wert,zpos+zk_wert,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
		     if (endprg) { *ok=1; *esc=1; }
		     xrad=xrad+xi_wert;
		     zpos=zpos+zk_wert;
		   } else
		   { /* G01 ABS */
		     endprg=Ziehe_Gerade(xrad,zpos,(short)(xi_wert/2)+xcorr,zk_wert+zcorr,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
		     if (endprg) { *ok=1; *esc=1; }
		     xrad=(short)(xi_wert/2)+xcorr;
		     zpos=zk_wert+zcorr;
		   }
	      }
	      else { *ok=0; endprg=1; }
	     break;
     /**************** G02 *******************/
     case 02:if (cncprg[ip].XI[0]!='I' && cncprg[ip].ZK[0]!='K')
	      {
		/* test ob == | != 90 */
		if (!strcmp(cncprg[ip+1].GM,"M99 "))
		    {
		      /* Mittelpunkt bestimmen */
		      Xpos=atoi(cncprg[ip+1].XI+1);
		      Zpos=atoi(cncprg[ip+1].ZK+1);
		      if (cncprg[ip+1].ZK[0]!='K' || cncprg[ip+1].XI[0]!='I')
			{
			  Meldung("Error in M99-Anweisung!"); Status("Fehler");
			  getch();
			  endprg=1; *ok=0;
			} else
			{
			  /* Kreisbogen(X1,Z1,X2,Z2,XM,ZM,1=G03 0=G02 */
			  if (inkrementell)
			    {
			      endprg=Ziehe_KreisNO90(xrad,zpos,xrad+xi_wert,zpos+zk_wert,xrad+Xpos,zpos+Zpos,0);
			      /*xrad=xrad+xi_wert;
			      zpos=zk_wert;*/
			    }
			    else {
				   endprg=Ziehe_KreisNO90(xrad,zpos,(short)(xi_wert/2),zk_wert,xrad+Xpos,zpos+Zpos,0);
				   /*xrad=(short)(xi_wert/2);
				   zpos=zk_wert;*/
				 }
			  ip++;
			}
		    }
		 else
		 {
		   /* kreisbogen a=90¯ */
		   if (inkrementell)
		     { /* G02 INC */
		       endprg=Ziehe_Kreis02(xrad,zpos,xrad+xi_wert,zpos+zk_wert,
							     prgconfig.wait_g01);
		       if (endprg) { *ok=1; *esc=1; }
		       xrad=xrad+xi_wert;
		       zpos=zpos+zk_wert;

		     } else
		     { /* G02 ABS */
		       endprg=Ziehe_Kreis02(xrad,zpos,(short)(xi_wert/2)+xcorr,zk_wert+zcorr,prgconfig.wait_g01);
		       if (endprg) { *ok=1; *esc=1; }
		       xrad=(short)(xi_wert/2)+xcorr;
		       zpos=zk_wert+zcorr;
		     }
		  } /* von if ==/!=90 */
	      } else { *ok=0; endprg=1; } /*von if !I & !K */
	     break;
     /**************** G03 *******************/
     case 03:if (cncprg[ip].XI[0]!='I' && cncprg[ip].ZK[0]!='K')
	      {
		/* test ob == | != 90 */
		if (!strcmp(cncprg[ip+1].GM,"M99 "))
		    {
		      /* Mittelpunkt bestimmen */
		      Xpos=atoi(cncprg[ip+1].XI+1);
		      Zpos=atoi(cncprg[ip+1].ZK+1);
		      if (cncprg[ip+1].ZK[0]!='K' || cncprg[ip+1].XI[0]!='I')
			{
			  Meldung("Error in M99-Anweisung!"); Status("Fehler");
			  getch();
			  endprg=1; *ok=0;
			} else
			{
			  /* Kreisbogen(X1,Z1,X2,Z2,XM,ZM,1=G03 0=G02 */
			  if (inkrementell)
			    endprg=Ziehe_KreisNO90(xrad,zpos,xrad+xi_wert,zpos+zk_wert,xrad+Xpos,zpos+Zpos,1);
			    else endprg=Ziehe_KreisNO90(xrad,zpos,(short)(xi_wert/2),zk_wert,xrad+Xpos,zpos+Zpos,1);
			  ip++;
			}
		    }
		 else
		 {
		   /* kreisbogen a=90¯ */
		   if (inkrementell)
		     { /* G03 INC */
		       endprg=Ziehe_Kreis03(xrad,zpos,xrad+xi_wert,zpos+zk_wert,
							     prgconfig.wait_g01);
		       if (endprg) { *ok=1; *esc=1; }
		       xrad=xrad+xi_wert;
		       zpos=zpos+zk_wert;

		     } else
		     { /* G03 ABS */
		       endprg=Ziehe_Kreis03(xrad,zpos,(short)(xi_wert/2)+xcorr,zk_wert+zcorr,prgconfig.wait_g01);
		       if (endprg) { *ok=1; *esc=1; }
		       xrad=(short)(xi_wert/2)+xcorr;
		       zpos=zk_wert+zcorr;
		     } /* von inkr | abs */
		 } /* von if a==90 */

	      } else { *ok=0; endprg=1; } /*von if !I & !K */
	     break;
     /**************** G04 *******************/
     case 4 :/* Verweilzeit */
	     if (cncprg[ip].XI[0]!='I')
	      {
		 za=0;
		 step=0;
		 do {
		      _dos_gettime(&time);
		      if (kbhit()) taste=getch();
		      if ((short)time.second!=za) { za=(short)time.second; step++; }
		    } while (step<(short)(xi_wert/100) && taste!=27);
	      }
	      break;
     /**************** G21 *******************/
     case 21:break; /* rem */
     /**************** G25 *******************/
     case 25: /* Unterprgr */
	     if (cncprg[ip].FLKT[0]=='L' && flkt_wert<max_line && sp<4)
	     {
	       stack[sp++]=ip;
	       ip=--flkt_wert;
	     } else
	     {
		Status("Error!");
		Meldung("Fehler im GOSUB-G27-Befehl!");
		*ok=0; endprg=1;
	     }break;
     /**************** G26 *******************/
     case 26: /* werkzeugwechsel */
	     if (cncprg[ip].FLKT[0]=='T' && flkt_wert>0 && flkt_wert<=max_tools)
	     {
	       /* Anzeige */
	       _settextposition(3,37);
	       _outtext(cncprg[ip].FLKT);
	       _outtext("        ");
	       Tool2SCR(cncprg[ip].FLKT);

	       Setzte_Stahl(xrad,zpos,(char)flkt_wert);
	       endprg=Ziehe_Gerade(xrad,zpos,xrad+xi_wert,zpos+zk_wert,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
	       xrad=xrad+xi_wert;
	       zpos=zpos+zk_wert;
	       xcorr=xi_wert;	    /* Korrekturwerte des Stahles muessen */
	       zcorr=zk_wert;	    /* in Abs. mit einberechnet werden	  */
	     } else { *ok=0; endprg=1; }
	     break;
     /**************** G27 *******************/
     case 27: /* goto */
	     if (cncprg[ip].FLKT[0]=='L' && flkt_wert<max_line)
		ip=--flkt_wert;
		else { *ok=0; endprg=1; }
	     break;
     /**************** G33 *******************/
     case 33: /* gewinde schneiden */
	     if (cncprg[ip].FLKT[0]=='K' && flkt_wert>=2 && flkt_wert<500)
	      {
		steigung=Unit2Pnt(flkt_wert);
		steigung=flkt_wert;
		if (inkrementell)
		   { /* INCR */
		     endprg=Gewinde(xrad,zpos,zpos+zk_wert,steigung,prgconfig.wait_g01+10000);
		     if (endprg) { *ok=1; *esc=1; }
		     zpos=zpos+zk_wert;
		   } else
		   { /* ABS */
		     endprg=Gewinde(xrad,zpos,zcorr+zk_wert,steigung,prgconfig.wait_g01+10000);
		     if (endprg) { *ok=1; *esc=1; }
		     zpos=zk_wert+zcorr;
		   }
	      }
	      else { *ok=0; endprg=1; }
	      break;
     /**************** G78 *******************/
     case 78: /* Gewindezyklus */
	     if (cncprg[ip].FLKT[0]=='K' && flkt_wert>=2 && flkt_wert<500)
	      {
		/* Start & Zielkoord */
		P1x=xrad; P1z=zpos;
		if (inkrementell)
		    {
		      P2x=xrad+xi_wert;
		      P2z=zpos+zk_wert;
		    } else {
			     P2x=(short)(xi_wert/2)+xcorr;
			     P2z=zk_wert+zcorr;
			   }
		/* Schritt & Steigung */
		step=h_wert;
		steigung=Unit2Pnt(flkt_wert);
		steigung=flkt_wert;

		/* Anfahren */
		if (step!=0)
		 if (P1x>P2x)
		    for (Xpos=P1x;Xpos-step>P2x && !endprg;Xpos-=step)
			{
			  endprg=Ziehe_Gerade(P1x,P1z,Xpos-step,P1z,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Gewinde(Xpos-step,P1z,P2z,steigung,prgconfig.wait_g01+10000);
			  if (!endprg)
			    endprg=Ziehe_Gerade(xrad,zpos,P1x,P2z,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P1x,P2z,P1x,P1z,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			}
		  else for (Xpos=P1x;Xpos+step<P2x && !endprg;Xpos+=step)
			{
			  endprg=Ziehe_Gerade(P1x,P1z,Xpos+step,P1z,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Gewinde(Xpos+step,P1z,P2z,steigung,prgconfig.wait_g01+10000);
			  if (!endprg)
			    endprg=Ziehe_Gerade(xrad,zpos,P1x,P2z,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P1x,P2z,P1x,P1z,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			 }

		/* letzter Schritt oder Schritt ohne Aufteilung*/
		if (!endprg)
		  endprg=Ziehe_Gerade(P1x,P1z,P2x,P1z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		if (!endprg)
		   endprg=Gewinde(P2x,P1z,P2z,steigung,prgconfig.wait_g01+20000);
		if (!endprg)
		  endprg=Ziehe_Gerade(xrad,zpos,P1x,P2z,
			 prgconfig.wait_g01+4000,schritt_g00,checkTasten);
		if (!endprg)
		  endprg=Ziehe_Gerade(P1x,P2z,P1x,P1z,
			 prgconfig.wait_g00,schritt_g00,checkTasten);

		if (endprg) { *ok=1; *esc=1; }
		xrad=P1x;
		zpos=P1z;
	      } break;
     /**************** G84 *******************/
     case 84: /* laengsdrehzyklus */
	      if (cncprg[ip].XI[0]!='I' && cncprg[ip].ZK[0]!='K')
	      {
		/* Start & Zielkoord */
		P1x=xrad; P1z=zpos;
		if (inkrementell)
		    {
		      P2x=xrad+xi_wert;
		      P2z=zpos+zk_wert;
		    } else {
			     P2x=(short)(xi_wert/2)+xcorr;
			     P2z=zk_wert+zcorr;
			   }
		/* Schritt */
		step=h_wert;

		/* Anfahren */
		if (step!=0)
		 if (P1x>P2x)
		    for (Xpos=P1x;Xpos-step>P2x && !endprg;Xpos-=step)
			{
			  endprg=Ziehe_Gerade(Xpos,P1z,Xpos-step,P1z,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(Xpos-step,P1z,Xpos-step,P2z,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(Xpos-step,P2z,Xpos-step,P1z,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			}
		  else for (Xpos=P1x;Xpos+step<P2x && !endprg;Xpos+=step)
			 {
			   endprg=Ziehe_Gerade(Xpos,P1z,Xpos+step,P1z,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			   if (!endprg)
			    endprg=Ziehe_Gerade(Xpos+step,P1z,Xpos+step,P2z,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			   if (!endprg)
			    endprg=Ziehe_Gerade(Xpos+step,P2z,Xpos+step,P1z,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			 }
		  else Xpos=P1x; /* von if step!=0 */

		/* letzter Schritt */
		if (!endprg)
		  endprg=Ziehe_Gerade(Xpos,P1z,P2x,P1z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		if (!endprg)
		  endprg=Ziehe_Gerade(P2x,P1z,P2x,P2z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		if (!endprg)
		  endprg=Ziehe_Gerade(P2x,P2z,P1x,P2z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		if (!endprg)
		  endprg=Ziehe_Gerade(P1x,P2z,P1x,P1z,
			 prgconfig.wait_g00,schritt_g00,checkTasten);

		if (endprg) { *ok=1; *esc=1; }
		xrad=P1x;
		zpos=P1z;
	      } break;
     /**************** G86 *******************/
     case 86: /* Austechzyklus */
	      if (cncprg[ip].XI[0]!='I' && cncprg[ip].ZK[0]!='K')
	      {
		/* Start & Zielkoord */
		P1x=xrad; P1z=zpos;
		if (inkrementell)
		    {
		      P2x=xrad+xi_wert;
		      P2z=zpos+zk_wert;
		    } else {
			     P2x=(short)(xi_wert/2)+xcorr;
			     P2z=zk_wert+zcorr;
			   }
		/* Schritt */
		step=h_wert;
		za=abs(P1z-P2z);
		if (za<step || step<10 || step>999)
		  *ok=0;
		  else
		  {
		    /* Anfahren */
		    if (P1z>P2z)
		      for (Zpos=P1z;Zpos>P2z && !endprg;Zpos-=step)
			{
			  endprg=Ziehe_Gerade(P1x,Zpos,P2x,Zpos,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P2x,Zpos,P1x,Zpos,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P1x,Zpos,P1x,Zpos-step,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			}
		     else for (Zpos=P1z;Zpos<P2z && !endprg;Zpos+=step)
			{
			  endprg=Ziehe_Gerade(P1x,Zpos,P2x,Zpos,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P2x,Zpos,P1x,Zpos,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P1x,Zpos,P1x,Zpos+step,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			}
		    /* letzter Schritt */
		    if (!endprg)
		      endprg=Ziehe_Gerade(P1x,Zpos,P1x,P2z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		    if (!endprg)
		      endprg=Ziehe_Gerade(P1x,P2z,P2x,P2z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		    if (!endprg)
		      endprg=Ziehe_Gerade(P2x,P2z,P1x,P2z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		    if (!endprg)
		      endprg=Ziehe_Gerade(P1x,P2z,P1x,P1z,
			 prgconfig.wait_g00,schritt_g00,checkTasten);

		    if (endprg) { *ok=1; *esc=1; }
		  }

		xrad=P1x;
		zpos=P1z;
	      }break;
     /**************** G88 *******************/
     case 88: /* plandrehzyklus */
	      if (cncprg[ip].XI[0]!='I' && cncprg[ip].ZK[0]!='K')
	      {
		/* Start & Zielkoord */
		P1x=xrad; P1z=zpos;
		if (inkrementell)
		    {
		      P2x=xrad+xi_wert;
		      P2z=zpos+zk_wert;
		    } else {
			     P2x=(short)(xi_wert/2)+xcorr;
			     P2z=zk_wert+zcorr;
			   }
		/* Schritt */
		step=h_wert;

		/* Anfahren */
		if (step!=0)
		 if (P1z>P2z)
		    for (Zpos=P1z;Zpos-step>P2z && !endprg;Zpos-=step)
			{
			  endprg=Ziehe_Gerade(P1x,Zpos,P1x,Zpos-step,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P1x,Zpos-step,P2x,Zpos-step,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P2x,Zpos-step,P1x,Zpos-step,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			}
		  else for (Zpos=P1z;Zpos+step<P2z && !endprg;Zpos+=step)
			{
			  endprg=Ziehe_Gerade(P1x,Zpos,P1x,Zpos+step,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P1x,Zpos+step,P2x,Zpos+step,
					 prgconfig.wait_g01,schritt_g01,checkTasten);
			  if (!endprg)
			    endprg=Ziehe_Gerade(P2x,Zpos+step,P1x,Zpos+step,
					 prgconfig.wait_g00,schritt_g00,checkTasten);
			}
		  else Zpos=P1z; /* von if step!=0 */

		/* letzter Schritt */
		if (!endprg)
		  endprg=Ziehe_Gerade(P1x,Zpos,P1x,P2z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		if (!endprg)
		  endprg=Ziehe_Gerade(P1x,P2z,P2x,P2z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		if (!endprg)
		  endprg=Ziehe_Gerade(P2x,P2z,P2x,P1z,
			 prgconfig.wait_g01,schritt_g01,checkTasten);
		if (!endprg)
		  endprg=Ziehe_Gerade(P2x,P1z,P1x,P1z,
			 prgconfig.wait_g00,schritt_g00,checkTasten);

		if (endprg) { *ok=1; *esc=1; }
		xrad=P1x;
		zpos=P1z;
	      } break;
     /**************** G90 *******************/
     case 90:{
	       /* Absoltut ein : NP = Pos des Drehmeissels */
	       inkrementell=0;
	       _settextposition(3,66);
	       _outtext("ABS");

	       /* Neuer NP setzten & akt. Koord. anpassen */
	       NeuerGrNP(xrad,zpos);
	       xrad=0;
	       zpos=0;
	     } break;
     /**************** G91 *******************/
     case 91:/* Inkrementell ein */
	     inkrementell=1;
	     break;
     /**************** G92 *******************/
     case 92:if (cncprg[ip].XI[0]!='I' && cncprg[ip].ZK[0]!='K')
	      {
		/* Absolut ein : NP = Parameter */
		inkrementell=0;
		_settextposition(3,66);
		_outtext("ABS");

		/* Neuer NP setzten & akt. Koord. anpassen */
		zw_buf1=tool; /* Werkzeug zwischenspeichern & loeschen */
		Setzte_Stahl(0,0,0);

		NeuerGrNP(xrad-(short)(xi_wert/2),zpos-zk_wert);
		xrad=(short)(xi_wert/2);
		zpos=zk_wert;

		tool=zw_buf1;
		Setzte_Stahl(xrad,zpos,tool);

	      } else { *ok=0; endprg=1; } /*von if !I & !K */
	     break;
     /**************** G94 *******************/
     case 94: break; /* zZ no Action */
     /**************** G95 *******************/
     case 95: break; /* zZ no Action */
     /**************** Error *******************/
     default:/* Unbekannter G-Befehl */
	     {
	       *ok=0;	 /* Fehlerflag */
	       endprg=1; /* Prg-Ende-Flag */
	     }
  }/* von switch m_wert */

  /* Returnwert */
  if (!*ok && endprg)
	    {
	      Status("E-Unterbrechung");
	      Meldung("G-Befehl in der aktuellen Zeile ist unbekannt oder unkorrekt!");
	    }

  return endprg;
}

void NeuerGrNP(short xr_mm,short zp_mm)
{
   short yp_p=Unit2Pnt(xr_mm),
	 xp_p=Unit2Pnt(zp_mm),
	 dx=xp_p,
	 dy=yp_p;

   grNPx+=dx;
   grNPy+=dy;
   _setvieworg(grNPx,grNPy);
}

char Check_FlagTasten(void)
{
  char taste,endprg=0;
  char meldung_zwischen[80],
       status_zwischen[30];

  if (kbhit())
    {
      taste=(char)getch();
      while (kbhit()) getch();

      strcpy(meldung_zwischen,meldung_str);
      strcpy(status_zwischen,status_str);

      switch(taste)
      {
	case 'm':menue=0; break;
	case 'n':{
		   trace=1;
		   _settextposition(4,37);
		   _outtext("Ein");
		 }break;
	case 'f':{
		   trace=0;
		   _settextposition(4,37);
		   _outtext("Aus");
		 }break;
	case 's':{
		   Meldung("Programmfortsetzung mit 'A' - Abbruch der Simulation mit ESC");
		   Status("Pause");
		   do {
			taste=(char)getch();
			if (taste==27) endprg=1;
		      } while (taste!='a' && !endprg);
		   Meldung(meldung_zwischen);
		   Status(status_zwischen);
		 }
		 break;
	case 27: endprg=1;
      } /* von switch */
    } /* von if taste */

  return endprg;
}

void ZeileAusgabe(short ip)
{

   if (ip!=0) SUBZeileAusgabe(ip-1,28);
	 else {
		_settextposition(28,1);
		_outtext("                                                                     ");
	      }
   SYS_PushColor();
   _settextcolor(2);
   SUBZeileAusgabe(ip,29);
   SYS_PopColor();
   if (ip<max_line-2) SUBZeileAusgabe(ip+1,30);
      else {
	     _settextposition(28,1);
	     _outtext("                                                                     ");
	   }
}

void SUBZeileAusgabe(short ip,char yp)
{
   char buf[105];
   _settextposition(yp,1);
   sprintf(buf,"N%03d G:%sX:%sZ:%sF:%sH:%s≥%s",ip,cncprg[ip].GM,
	 cncprg[ip].XI,cncprg[ip].ZK,cncprg[ip].FLKT,cncprg[ip].H,cncprg[ip].BEM);
   _outtext(buf);
}

char SucheM30(void)
{
  char flag=0;
  short z;

  for (z=0;z<max_line && !flag;z++)
    if (!strcmp(cncprg[z].GM,"M30 ")) flag=1;

  if (!flag)
    {
      Status("Ende wegen Fehler");
      Meldung("Endbefehl M30 wurde nicht gefunden oder nicht korrekt eingegeben!");
    }

  return flag;
}

/*** Bewegungsalgr. ***/

/* Alle Routinen die hoeher liegen arbeiten in 1/100mm,ab hier wird in
   Punkte gerechnet */

char Ziehe_KreisNO90(short xr1,short zp1,short xr2,short zp2,short xrm,short zpm,char g03g02)
{
  double sx=(double)zp1,sy=(double)xr1,
	 ex=(double)zp2,ey=(double)xr2,
	 mx=(double)zpm,my=(double)xrm;
  double dx,dy,w,cosw,sinw;
  double rad1,rad2,stw,endw,step=(double)schritt_g02;
  char quadr1,quadr2,endprg=0;
  short x,y;

  dx=sx-mx;
  dy=sy-my;
  rad1=sqrt(dx*dx+dy*dy);
  cosw=dx/rad1;
  sinw=dy/rad1;
  stw=arc_wi(sinw,cosw,&quadr1);

  dx=ex-mx;
  dy=ey-my;
  rad2=sqrt(dx*dx+dy*dy);
  cosw=dx/rad2;
  sinw=dy/rad2;
  endw=arc_wi(sinw,cosw,&quadr2);

  if (quadr1!=quadr2) { /* Routine zZ noch nicht in Funktion */
			Meldung("M99 Quadranten-Fehler!");
			Status("Fehler!");
			getch();
			endprg=1;
		      }

  if (fabs(fabs(rad1)-fabs(rad2))>160)
    {
      Meldung("M99-0,8mm-Toleranz Åberschritten");
      Status("Fehler");
      getch();
      endprg=1; /* Nur zZ */
    }

  if (!endprg)
    {
      if (g03g02)
	{ /* G02 */
	   if (endw<stw) stw-=360.0;
	   for(w=stw;w<endw && !endprg;w+=step)
	      {
		x=(short)(mx+rad1*cos(w*deg));
		y=(short)(my+rad1*sin(w*deg));
		Setzte_Stahl(y,x,tool);
		Koordinaten(y,x);
		xrad=y;
		zpos=x;
		Wait(prgconfig.wait_g01);
		endprg=Check_FlagTasten();
	      }
	} else { /* g03 */
		 if (endw>stw) endw-=360.0;
		 for(w=stw;w>endw && !endprg;w-=step)
		  {
		    x=(short)(mx+rad1*cos(w*deg));
		    y=(short)(my+rad1*sin(w*deg));
		    Setzte_Stahl(y,x,tool);
		    Koordinaten(y,x);
		    xrad=y;
		    zpos=x;
		    Wait(prgconfig.wait_g01);
		    endprg=Check_FlagTasten();
		  }
	       } /* von g03 */
    } /* von if !endprg */

    if (!endprg)
       {
	 Setzte_Stahl(xr2,zp2,tool);
	 xrad=xr2;
	 zpos=zp2;
	 Koordinaten(xr2,zp2);
       }

  return endprg;
}

/* Eingabe Start & Endpunkt => ergebnis Rechtw. Kreisboden von 1 -> 2*/
char Ziehe_Kreis03(short xr0,short zp0,short xr1,short zp1,short warte)
{
  double mp_x,mp_y,delta_x,delta_y,winkel,startw,rad,x,y;
  double cosw,sinw;
  char endprg=0,q;

  mp_x=(zp0-xr1+xr0+zp1)/2;
  mp_y=(xr0+zp1+xr1-zp0)/2;

  delta_x=(double)zp0-mp_x;
  delta_y=(double)xr0-mp_y;
  rad=sqrt(delta_x*delta_x+delta_y*delta_y);

  cosw=((double)zp0-mp_x)/rad;
  sinw=((double)xr0-mp_y)/rad;
  startw=arc_wi(sinw,cosw,&q);

  for(winkel=startw;winkel<=startw+89.9 && !endprg;winkel+=schritt_g02)
    {
      x=mp_x+rad*cos(winkel*deg);
      y=mp_y+rad*sin(winkel*deg);
      Setzte_Stahl((short)y,(short)x,tool);
      Koordinaten((short)y,(short)x);
      Wait(warte);
      endprg=Check_FlagTasten();
    }

  if (!endprg)
    {
      Setzte_Stahl(xr1,zp1,tool);
      Koordinaten(xr1,zp1);
      xrad=xr1;
      zpos=zp1;
    }
  return endprg;
}

char Ziehe_Kreis02(short xr0,short zp0,short xr1,short zp1,short warte)
{
  double mp_x,mp_y,dx,dy,winkel,stw,rad,x,y;
  double cosw,sinw;
  char endprg=0,q;

  mp_x=(zp1-xr0+xr1+zp0)/2.0;
  mp_y=(xr1+zp0+xr0-zp1)/2.0;
  dx=(double)zp0-mp_x;
  dy=(double)xr0-mp_y;
  rad=sqrt(dx*dx+dy*dy);

  cosw=((double)zp0-mp_x)/rad;
  sinw=((double)xr0-mp_y)/rad;
  stw=arc_wi(sinw,cosw,&q);

  for(winkel=stw;winkel>stw-89.9;winkel-=schritt_g02)
    {
      x=mp_x+rad*cos(winkel*deg);
      y=mp_y+rad*sin(winkel*deg);
      Setzte_Stahl((short)y,(short)x,tool);
      Koordinaten((short)y,(short)x);
      Wait(warte);
      endprg=Check_FlagTasten();
    }

  if (!endprg)
    {
      Setzte_Stahl(xr1,zp1,tool);
      Koordinaten(xr1,zp1);
      xrad=xr1;
      zpos=zp1;
    }

  return endprg;
}

char Ziehe_Gerade(short xr0_mm,short zp0_mm,short xr1_mm,short zp1_mm,
		  short warte,short step,char check)
{
  short zp,ya,xr;
  char endprg=0;

  if (abs(zp0_mm-zp1_mm)>=50 && zp0_mm<zp1_mm)
    for (zp=zp0_mm;zp<zp1_mm && !endprg;zp+=step)
       {
	 ya=(short)((((float)(xr1_mm-xr0_mm))/((float)(zp1_mm-zp0_mm)))*
			      ((float)(zp-zp0_mm)));
	 xr=ya+xr0_mm;
	 Setzte_Stahl(xr,zp,tool);
	 Koordinaten(xr,zp);
	 Wait(warte);
	 if (check) endprg=Check_FlagTasten();
       }

  if (abs(zp0_mm-zp1_mm)>=50 && zp0_mm>zp1_mm)
    for (zp=zp0_mm;zp>zp1_mm && !endprg;zp-=step)
      {
	ya=(short)((((float)(xr1_mm-xr0_mm))/((float)(zp1_mm-zp0_mm)))*
			      ((float)(zp-zp0_mm)));
	xr=ya+xr0_mm;
	Setzte_Stahl(xr,zp,tool);
	Koordinaten(xr,zp);
	Wait(warte);
	if (check) endprg=Check_FlagTasten();
      }

  if (abs(zp0_mm-zp1_mm)<50 && xr1_mm<xr0_mm)
    for (xr=xr0_mm,zp=zp0_mm;xr>xr1_mm && !endprg;xr-=step)
      {
	Setzte_Stahl(xr,zp,tool);
	Koordinaten(xr,zp);
	Wait(warte);
	if (check) endprg=Check_FlagTasten();
      }

  if (abs(zp0_mm-zp1_mm)<50 && xr1_mm>xr0_mm)
    for (xr=xr0_mm,zp=zp0_mm;xr<xr1_mm && !endprg;xr+=step)
      {
	Setzte_Stahl(xr,zp,tool);
	Koordinaten(xr,zp);
	Wait(warte);
	if (check) endprg=Check_FlagTasten();
      }

  /* Um Fehler beim Ende der Schleifenbearbeitung zu korrigieren */
  if (!endprg)
    {
      Setzte_Stahl(xr1_mm,zp1_mm,tool);
      Koordinaten(xr1_mm,zp1_mm);
    }
  return endprg;
}


char Gewinde(short Xrad,short Zpos1,short Zpos2,short steigung,short warte)
{
   char endprg=0;
   short za;

   if (Zpos1<Zpos2)
    for (za=Zpos1;za<Zpos2 && !endprg;za+=steigung)
	{
	  zpos=za;
	  Setzte_Stahl(Xrad,za,tool);
	  Koordinaten(Xrad,za);
	  Wait(warte);
	  endprg=Check_FlagTasten();
	}
    else for (za=Zpos1;za>Zpos2 && !endprg;za-=steigung)
	   {
	     zpos=za;
	     Setzte_Stahl(Xrad,za,tool);
	     Koordinaten(Xrad,za);
	     Wait(warte);
	     endprg=Check_FlagTasten();
	   }
   xrad=Xrad;

   return endprg;
}


/* Diese Routine gibt den, durch den COS & SIN exakt definierten Winkel */
/* zurueck: 0 - 359.99¯ 						*/
double arc_wi(double sinw,double cosw,char *quadrant)
{
  double cosw1=acos(cosw)/deg,
	 cosw2=-acos(cosw)/deg,
	 endw;
  char quadr;

  if (sinw!=0.0 && cosw!=0.0)
      {
	/* Feststellen des Quadranten */
	if (sinw>0 && cosw>0) quadr=1;
	if (sinw>0 && cosw<0) quadr=2;
	if (sinw<0 && cosw<0) quadr=3;
	if (sinw<0 && cosw>0) quadr=4;

	/* Gegenwinkel in [0-360¯] bringen */
	if (cosw2<0.0)	cosw2+=360.0;
	if (cosw2>=360.0) cosw2-=360.0;

	/* Winkel & -Winkel haben den gleichen COS */
	if (quadr==1 || quadr==2) endw=(cosw1<cosw2) ? cosw1:cosw2;
			     else endw=(cosw1>cosw2) ? cosw1:cosw2;
     }
     else  /* Wenn Punkt aus Achse -> Sonderbehandlung */
	   {
	     if (cosw==0.0) endw=asin(sinw)/deg;
		       else endw=acos(cosw)/deg;
	     /* Winkel wieder in [0-360¯] zwingen */
	     if (endw<0.0) endw+=360.0;
	   }
  /* Quadrant des Endwinkels feststellen.
     Dies umstaendliche Technik muss benutzt werden, da bei der Sonder-
     behandlung (0,90,180,270¯) der Quadrant nicht vorberechnet wird */
  if (endw<90)		     *quadrant=1;
  if (endw>=90 && endw<180)  *quadrant=2;
  if (endw>=180 && endw<270) *quadrant=3;
  if (endw>=270)	     *quadrant=4;
  *quadrant=0; /* Definition des quadranten ???? */

  return endw;
}




/* SUBFunctionen & Vorbereitung */

void Automatisch_Ankratzen(void)
{
   short w=prgconfig.wait_g00;

   Meldung("StÅck wird automatisch angefahren.");
   Status("Auto Nullpunkt");
   Setzte_Stahl(radius+1000,laenge+1000,1);
   tool=1;
   /* Plan ankratzen */
   Ziehe_Gerade(radius+1000,laenge+1000,radius-500,laenge+1000,w,schritt_g00,!checkTasten);
   Ziehe_Gerade(radius-500,laenge+1000,radius-500,laenge+50,w,schritt_g00,!checkTasten);
   Ziehe_Gerade(radius-500,laenge+100,radius-500,laenge+1000,w,schritt_g00,!checkTasten);
   Ziehe_Gerade(radius-500,laenge+1000,radius+1000,laenge+1000,w,schritt_g00,!checkTasten);

   /* Seite ankr. */
   Ziehe_Gerade(radius+1000,laenge+1000,radius+1000,laenge-500,w,schritt_g00,!checkTasten);
   Ziehe_Gerade(radius+1000,laenge-500,radius+50,laenge-500,w,schritt_g00,!checkTasten);
   Ziehe_Gerade(radius+50,laenge-500,radius+1000,laenge-500,w,schritt_g00,!checkTasten);
   Ziehe_Gerade(radius+1000,laenge-500,radius+1000,laenge+1000,w,schritt_g00,!checkTasten);

   /* Zur Ecke fahren */
   Ziehe_Gerade(radius+1000,laenge+1000,radius+50,laenge+50,w,schritt_g00,!checkTasten);
   Ziehe_Gerade(radius+50,laenge+50,radius+100,laenge+100,w,schritt_g00,!checkTasten);

   Koordinaten(radius+100,100);
}

char Manuell_Nullpunkt_festlegen(void)
{
   int taste;

   Meldung("Bitte wÑhlen Sie Ihren Startpunkt: (Ctrl+) Cursortasten & Enter");
   Status("Manuell Startpunkt");
   zpos=100;
   xrad=radius+100;

   /* Stahl bewegen */
   Setzte_Stahl(xrad,zpos,1);

   do{
	taste=getch();
	if (taste==0) taste=getch();

	switch(taste)
	{
	  case 72:/* rauf */
		  if (xrad>10)
		    xrad-=10;
		  break;
	  case 80:/* runter */
		  if (xrad<radius+2000)
		    xrad+=10;
		  break;
	  case 77:/* rechts */
		  if (zpos<6000)
		    zpos+=10;
		  break;
	  case 75:/* links */
		  if (zpos>-((laenge>6000) ? 6000:laenge))
		    zpos-=10;
		  break;
	  case 115:/* ctrl+links */
		  if (zpos>-((laenge>5750) ? 5750:laenge))
		    zpos-=250;
		  break;
	  case 116:/* ctrl+rechts */
		  if (zpos<6000)
		    zpos+=250;
		   break;
	  case 141:/* crtl+rauf */
		  if (xrad>0)
		    xrad-=250;
		   break;
	  case 145:/* crtl+runter */
		  if (xrad<radius+2000)
		    xrad+=250;
	} /* von switch taste */

	Koordinaten(xrad,zpos);
	Setzte_Stahl(xrad,zpos,1);

     } while (taste!=27 && taste!=13);

   if (taste==27) return 1;
     else return 0;
}

/* Koordinaten ausgeben */
void Koordinaten(short xrad_mm,short zpos_mm)
{
   char buf_x[35],buf_y[35];

   sprintf(buf_x,"%+06.2f mm",(float)(xrad_mm*2)/100.0F);
   sprintf(buf_y,"%+06.2f mm",(float)zpos_mm/100.0F);

   SYS_PushColor();
   _settextcolor(3);
   _settextposition(3,10);
   _outtext(buf_x);
   _settextposition(4,10);
   _outtext(buf_y);
   SYS_PopColor();

   /* Das gleicher fuer den 2. Monitor */
   Koord2SCR(xrad_mm,zpos_mm);
}

/* Gibt Status aus*/
void Status(char *text)
{
  SYS_PushColor();
  _settextposition(6,53);
  _outtext("                          ");
  _settextposition(6,53);
  _settextcolor(3);
  _outtext(text);
  SYS_PopColor();
  strcpy(status_str,text);

  Status2SCR(text);
}

/* Gibt Meldung aus */
void Meldung(char *text)
{
  SYS_PushColor();
  _settextposition(7,12);
  _outtext("                                                                 ");
  _settextposition(7,12);
  _settextcolor(4);
  _outtext(text);
  SYS_PopColor();
  strcpy(meldung_str,text);

  Meldung2SCR(text);
}

/* Rechnet von 1/100mm in Pixel um */
short Unit2Pnt(short mm_div_100)
{
   short ret=(short)( ((float)mm_div_100/100.0F)*3.0F );
   return ret;
}

/* Grafikbildschirm einrichten */
void SimBildschirmmaske(void)
{
  char buf_l[20],buf_d[20];

  sprintf(buf_l,"%6.2f",(float)laenge/100.0F);
  sprintf(buf_d,"%6.2f",(float)durchm/100.0F);

  _setcolor(7);
  _moveto(0,431);
  _lineto(640,431);
  _settextcolor(3);
  _settextposition(1,1);
  _outtext(" Simulator:   ≥ Pausefunktion: ");
  _settextcolor(5); _outtext("S"); _settextcolor(3);
  _outtext("top  St");
  _settextcolor(5); _outtext("a"); _settextcolor(3);
  _outtext("rt ≥ Trace: TrO");
  _settextcolor(5); _outtext("N"); _settextcolor(3);
  _outtext(" TrOF");
  _settextcolor(5); _outtext("F"); _settextcolor(3);
  _outtext(" ≥ Ende: ");
  _settextcolor(5); _outtext("ESC");
  _moveto(0,15);
  _lineto(640,15);
  _settextcolor(3);
  _settextposition(3,1);
  _outtext("X-Drm Ì:                  Werkzeug: T01 (Ref)        Koordinaten:INC \n");
  _outtext("Z-Pos  :                  Trace   : Aus \n");
  _outtext("\nWerkstÅck: l=");
  _outtext(buf_l);
  _outtext("mm  Ì=");
  _outtext(buf_d);
  _outtext("mm            Status:\nMeldung  :");

  _moveto(0,431);
  _lineto(640,431);

  Zeichne_stueck();

  /* Graphicfenster */
  _setviewport(0,115,639,430);
  _setvieworg(0,240);
  /* Maschinen-NP */
  _setcolor(4);
  _ellipse(_GFILLINTERIOR,2,2,-2,-2);
}

void Zeichne_stueck(void)
{
  short hoehe_pt,laenge_pt,start_pt,za;

  hoehe_pt=(short)((float)durchm*3/100);
  laenge_pt=(short)((float)laenge*3/100);
  start_pt=(short)((float)idurchm*3/100);

  _setvieworg(0,0);
  _setcolor(2);
  /* Untere Haelfte */
  for (za=0;za<laenge_pt;za++)
    {
      _moveto(za,240+(short)(start_pt/2));
      _lineto(za,240+(short)(hoehe_pt/2)-1);
    }
  _moveto(0,240+(short)(start_pt/2));
  _lineto(0,240);
  _moveto(laenge_pt-1,240+(short)(start_pt/2));
  _lineto(laenge_pt-1,240);

  /* Obere Haelfte */
  _moveto(0,240);
  _lineto(0,240-(short)(hoehe_pt/2)+1);
  _lineto(laenge_pt-1,240-(short)(hoehe_pt/2)+1);
  _lineto(laenge_pt-1,240);
  _moveto(0,240-(short)(start_pt/2));
  _lineto(laenge_pt-1,240-(short)(start_pt/2));

  /* Mittelaxe */
  _setcolor(4);
  _setlinestyle(0xff);
  _moveto(0,240);
  _lineto(laenge_pt-1,240);
  _setlinestyle(0xffff);
}

/* Durchmesser & Laenge eingeben & umwandeln */
char Hole_DMuLNG(void)
{
  struct LE_parameter edt={NULL,"0123456789 ",36,0,0,0,0,0,0,0,0,1,0,0};
  char feld=1,ret,zw_dm[6],zw_lng[6],raus=0,zw_idm[6];

  SYS_PushColor();
  _settextcolor(10);
  _setbkcolor(4);
  TW_Open_Window(18,14,44,7, TW_RAHMEN_S1,TW_TITEL_OBEN,15,"Simulation");
  _outtext("Programmsimulation:\nÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕ\nBitte WerkstÅckdaten eingeben:\n\n");
  _outtext("Durchmesser (1/100 mm : 100 =1 mm):");
  _outtext(st_durchmesser);
  _outtext("\nLÑnge       (1/100 mm)            :");
  _outtext(st_laenge);
  _outtext("\nInnendurchmesser                  :");
  _outtext(st_innendm);

  strcpy(zw_dm,st_durchmesser);
  strcpy(zw_lng,st_laenge);
  strcpy(zw_idm,st_innendm);

  do {
	/* Eing. & Test */
	SYS_CursorOn();
	/* eventl neue Werte ausgeben */
	_settextposition(5,36);
	_outtext(st_durchmesser);
	_settextposition(6,36);
	_outtext(st_laenge);

	do {
	     /* Eingabeschleife */

	     switch(feld)
	     {
	       case 1:
		{
		  edt.y=5;
		  edt.raus_oben=0;
		  edt.raus_unten=1;
		  edt.editstr=st_durchmesser;
		  ret=LE_LineEdit(&edt);
		  if (ret==LE_UNTEN_RAUS || ret==LE_OK)
		    {
		      feld=0;
		      ret=0;
		    }
		} break;
	       case 0:
		{
		  edt.y=6;
		  edt.raus_oben=1;
		  edt.raus_unten=1;
		  edt.editstr=st_laenge;
		  ret=LE_LineEdit(&edt);
		  if (ret==LE_OBEN_RAUS)
		    {
		      feld=1;
		      ret=0;
		    }
		  if (ret==LE_UNTEN_RAUS || ret==LE_OK)
		    {
		      feld=2;
		      ret=0;
		    }
		} break;
	       case 2:
		{
		  edt.y=7;
		  edt.raus_oben=1;
		  edt.raus_unten=0;
		  edt.editstr=st_innendm;
		  ret=LE_LineEdit(&edt);
		  if (ret==LE_OBEN_RAUS)
		    {
		      feld=0;
		      ret=0;
		    }
		} break;
	     } /* von switch */

	   } while (ret!=LE_ESC && ret!=LE_OK); /* Eingabeschleife */

       SYS_CursorOff();
       if (ret==LE_ESC)
	 {
	   /* bei ESC ->UNDO */
	   strcpy(st_durchmesser,zw_dm);
	   strcpy(st_laenge,zw_lng);
	   strcpy(st_innendm,zw_idm);
	   TW_Close_Window();
	   SYS_PopColor();
	   return 1;
	 }
	 else {
		/* Zahlen umwandeln & testen */
		durchm=atoi(st_durchmesser);
		idurchm=atoi(st_innendm);
		radius=(short)(durchm/2);
		laenge=atoi(st_laenge);
		sprintf(st_durchmesser,"%-05d",durchm);
		sprintf(st_innendm,"%-05d",idurchm);
		sprintf(st_laenge,"%-05d",laenge);

		raus=1;
		if (laenge<1000 || laenge>17000 || durchm>8000 || durchm<300 ||
		    idurchm>durchm-700 || (idurchm<1000 && idurchm!=0) || idurchm>15000)
		    {
		      _settextcolor(4);
		      _setbkcolor(10);
		      TW_Open_Window(10,10,45,6, TW_RAHMEN_S1,TW_NO_TITEL,0,NULL);
		      _outtext("          ***** ACHTUNG *****\n");
		      _outtext(" Grenzwerte: Durchmesser: 300 bis 8000\n");
		      _outtext("                         (3 mm - 80 mm)\n");
		      _outtext("             LÑnge      : 1000 bis 17000\n");
		      _outtext("                         (10 mm - 170 mm)\n");
		      _outtext("             InnenÌ     : 0 / 1000 bis 15000\n");
		      _outtext("                         (InnenÌ < AussenÌ)");
		      getch();
		      TW_Close_Window();
		      _settextcolor(10);
		      _setbkcolor(4);
		      raus=0;
		    }
	      }
     } while (!raus);

  TW_Close_Window();
  return 0;
}

/* Simulator initialisieren */
void Init_Simulator(void)
{
   char za;

   /* Variabeln initialisieren */
   /* Spritevariabeln:unmoegliche Werte=1x setzten */
   old_tool=99;

   inkrementell=1;
   trace=0;
   menue=0;
   xcorr=0;
   zcorr=0;
   if (!_setvideomode(_VRES16COLOR))
    {
      printf("VGAerror!\n");
      exit(2);
    }

   ip=0;		 /* Instruction Pointer */
   sp=0;		 /* Stack Pointer */
   for (za=0;za<6;za++)  /* Stack fuer Unterprogramme */
       stack[za]=null;

   tool=1;  /* Nullpunkt mit Referenzwerkzeug anfahren */
}

/* Stahl bewegen */
void Setzte_Stahl(short xrad_mm,short zpos_mm,char werkz)
{
    short xp=Unit2Pnt(zpos_mm),
	  yp=Unit2Pnt(xrad_mm);

    if (old_tool==99)
	{
	  /* erste Mal zeichnen */
	  xpt=xp;
	  ypt=yp;
	  old_tool=werkz;
	  tool=werkz;

	  Zeichne_Stahl(xp,yp,tool,15);
	}
      else
	{
	  /* Bewegen */
	  if (xp!=xpt || ypt!=yp || old_tool!=werkz) /* Um das Flackern zu vermindern */
	    {
	      Zeichne_Stahl(xpt,ypt,old_tool,0);
	      Zeichne_Stahl(xp,yp,werkz,15);
	    }
	  old_tool=werkz;
	  xpt=xp;
	  ypt=yp;

	  tool=werkz;/* fuer andere Routinen */
	}
}

void Wait(short w)
{
  short za,xyz;

  for (za=0;za<w;za++) xyz=za+w; /* Optimierung durch Kompiler verhindern */
}

/* Stahl Nummer "tool" zeichen */
void Zeichne_Stahl(short xp,short yp,char tool,short color)
{
   _setcolor(color);
   switch(tool)
   {
      case 1:/* rechter Seitenstahl */
	  {
	    _moveto(xp,yp);
	    _lineto(xp+0,yp+18);
	    _lineto(xp+18,yp+27);
	    _lineto(xp+18,yp+10);
	    _lineto(xp,yp);

	    _moveto(xp+6,yp+21);
	    _lineto(xp+6,yp+50);
	    _moveto(xp+18,yp+14);
	    _lineto(xp+30,yp+18);
	    _lineto(xp+30,yp+50);
	  }
	  break;
      case 2:/* linker Seitenstahl */
	  {
	    _moveto(xp+42,yp);
	    _lineto(xp+24,yp+12);
	    _lineto(xp+24,yp+30);
	    _lineto(xp+42,yp+18);
	    _lineto(xp+42,yp);

	    _moveto(xp+36,yp+21);
	    _lineto(xp+36,yp+50);
	    _moveto(xp+24,yp+15);
	    _lineto(xp+12,yp+21);
	    _lineto(xp+12,yp+50);
	  }
	  break;
      case 3:/* neutraler Drehstahl */
	  {
	    _moveto(xp+21,yp);
	    _lineto(xp+12,yp+16);
	    _lineto(xp+21,yp+32);
	    _lineto(xp+30,yp+16);
	    _lineto(xp+21,yp);

	    _moveto(xp+12,yp+19);
	    _lineto(xp+7,yp+24);
	    _lineto(xp+7,yp+50);
	    _moveto(xp+30,yp+19);
	    _lineto(xp+35,yp+24);
	    _lineto(xp+35,yp+50);
	  }
	  break;
      case 4:/* Gewindestahl aussen rechts */
	  {
	    /*_setpixel(xp+6,yp);*/

	    _moveto(xp+6,yp);
	    _lineto(xp+12,yp+9);
	    _lineto(xp+18,yp+9);
	    _lineto(xp+24,yp+12);
	    _lineto(xp+36,yp+12);
	    _lineto(xp+36,yp+50);

	    _moveto(xp+6,yp);
	    _lineto(xp-0,yp+10);
	    _lineto(xp-5,yp+10);
	    _lineto(xp-5,yp+50);
	  }
	  break;
      case 5:/* Stecher fuer Seegereinstiche */
	  {
	    _moveto(xp,yp+50);
	    _lineto(xp,yp-30);
	    _lineto(xp+5,yp-30);
	    _lineto(xp+5,yp-24);
	    _lineto(xp+9,yp-21);
	    _lineto(xp+9,yp-15);
	    _lineto(xp+30,yp-5);
	    _lineto(xp+30,yp+50);
	  }
	  break;
      case 6:/* Abstechklinge HSS */
	  {
	    _moveto(xp,yp+50);
	    _lineto(xp,yp);
	    _lineto(xp+10,yp);
	    _lineto(xp+10,yp+50);
	    _moveto(xp,yp+25);
	    _lineto(xp+10,yp+25);
	  }
	  break;
      case 7:/* Gewindestahl innen rechts */
	  {
	    _moveto(xp-60,yp+30);
	    _lineto(xp-54,yp+21);
	    _lineto(xp-48,yp+21);
	    _lineto(xp-42,yp+15);
	    _lineto(xp+60,yp+15);

	    _moveto(xp-60,yp+30);
	    _lineto(xp-66,yp+21);
	    _lineto(xp-75,yp+21);
	    _lineto(xp-75,yp+0);
	    _lineto(xp-69,yp-6);
	    _lineto(xp-69,yp-9);
	    _lineto(xp+60,yp-9);
	  }
	  break;
      case 8:/* Innendrehstahl */
	  {
	    _moveto(xp-60,yp+30);
	    _lineto(xp-60,yp+12);
	    _lineto(xp-42,yp+2);
	    _lineto(xp-42,yp+21);
	    _lineto(xp-60,yp+30);

	    _moveto(xp-42,yp+15);
	    _lineto(xp+30,yp+15);
	    _moveto(xp-54,yp+9);
	    _lineto(xp-54,yp-9);
	    _lineto(xp+30,yp-9);
	  }
   }
}

void DemoText(void)
{
  time_t t1,t2,t3;

  _settextposition(26,1);
  _outtext("EMCO-Simulator / CNC-Projekt der StÑdtischen Technischen Schule Eupen 1991/92");
  t1=time(&t3);
  t2=time(&t3);
  while ((t2-t1)<10) t2=time(&t3);
  _settextposition(26,1);
  _outtext("                                                                             ");
}


/* Vernichtet alle Leerzeichne in einem String, die Laenge bleibt erhalten */
/* max 2 Leerzeichen hintereinander */
void ElimSpaces(char *str)
{
  char pos,za,laenge=(char)(strlen(str)-1);

  for (pos=0;pos<=laenge;pos++)
      if (str[pos]==' ')
	{
	  for (za=pos;za<laenge;za++)
	    str[za]=str[za+1];
	  str[laenge]=' ';
	}
}

/****************************************************************
WERKZEUGBILDSCHIRM ( wird vom Editor aufgerufen )
*****************************************************************/
void Werkzeuge_darstellen(void)
{
   /* Textscreen retten */
   TW_Open_Window(2,2,78,23, TW_RAHMEN_NON,TW_NO_TITEL,0,NULL);
   _setvideomode(_MAXRESMODE);
   /*_setvideomode(_VRES16COLOR);*/

   /* Bildschirm */
   _settextposition(1,1);
   _setcolor(2);
   _outtext("Werkzeuge:\nƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ");
   _setcolor(16);

   /* T01 */
   Zeichne_Stahl(40,40,1,7);
   _settextcolor(15);
   _settextposition(7,7);
   _outtext("T01\n    rechter\n  Seitenstahl");
   _settextcolor(13);
   _settextposition(10,1);
   _outtext("(Referenzstahl)");

   /* T02 */
   Zeichne_Stahl(200,40,2,7);
   _settextcolor(15);
   _settextposition(7,28);
   _outtext("T02");
   _settextposition(8,26);
   _outtext("rechter");
   _settextposition(9,24);
   _outtext("Seitenstahl");
   _settextcolor(13);
   _settextposition(10,25);
   _outtext("Korrektur:");
   _settextposition(11,24);
   _outtext("X=0 Z=+14mm");

   /* T03 */
   Zeichne_Stahl(380,40,3,7);
   _settextcolor(15);
   _settextposition(7,50);
   _outtext("T03");
   _settextposition(8,47);
   _outtext("neutraler");
   _settextposition(9,47);
   _outtext("Drehstahl");
   _settextcolor(13);
   _settextposition(10,47);
   _outtext("Korrektur:");
   _settextposition(11,47);
   _outtext("X=0 Z=+7mm");

   /* T04 */
   Zeichne_Stahl(550,40,4,7);
   _settextcolor(15);
   _settextposition(7,70);
   _outtext("T04");
   _settextposition(8,66);
   _outtext("Gewindestahl");
   _settextposition(9,66);
   _outtext("au·en rechts");
   _settextcolor(13);
   _settextposition(10,67);
   _outtext("Korrektur:");
   _settextposition(11,67);
   _outtext("X=0 Z=+2mm");

   /* T05 */
   Zeichne_Stahl(45,250,5,7);
   _settextcolor(15);
   _settextposition(20,7);
   _outtext("T05\n   Stechstahl\n\n");
   _settextcolor(13);
   _outtext("   Korrektur:\n");
   _outtext("  X=-10mm Z=0\n");
   _settextcolor(12);
   _outtext("Breite   : 1,2mm\n");
   _outtext("max Tiefe: 1,5mm\n");

   /* T06 */
   Zeichne_Stahl(210,250,6,7);
   _settextcolor(15);
   _settextposition(20,26);
   _outtext("T06");
   _settextposition(21,21);
   _outtext("Abstechklinge");
   _settextposition(22,25);
   _outtext("(HSS)");
   _settextcolor(13);
   _settextposition(23,22);
   _outtext("Korrektur:");
   _settextposition(24,24);
   _outtext("X=0 Z=0");

   /* T07 */
   Zeichne_Stahl(390,270,7,7);
   _settextcolor(15);
   _settextposition(20,47);
   _outtext("T07");
   _settextposition(21,43);
   _outtext("Gewindestahl");
   _settextposition(22,43);
   _outtext("innen rechts");
   _settextcolor(13);
   _settextposition(23,44);
   _outtext("Korrektur:");
   _settextposition(24,41);
   _outtext("X=+10mm Z=-20mm");

   /* T08 */
   Zeichne_Stahl(570,270,8,7);
   _settextcolor(15);
   _settextposition(20,70);
   _outtext("T08");
   _settextposition(21,67);
   _outtext("Innendreh-");
   _settextposition(22,70);
   _outtext("stahl");
   _settextcolor(13);
   _settextposition(23,67);
   _outtext("Korrektur:");
   _settextposition(24,65);
   _outtext("X=+10mm Z=-20mm");

   _settextcolor(15);
   _settextposition(28,1);
   /*_outtext("Zusatz:Bohrer        T09 & F-Feld: Durchmesser (Korrektur: Rad=0 & Zpos)\n");*/
   /*_outtext("       Einstechstahl T10           Breite      (           Rad=0 & Zpos=FWert)\n");*/
   /*_outtext("       Ausreibstahl  T11           Durchmesser (           wie Bohrer)");*/

   getch();

   /* Textscreen zurueckholen */
   _setvideomode(_TEXTC80);
   TW_Close_Window();
   SYS_CursorOff();
}
