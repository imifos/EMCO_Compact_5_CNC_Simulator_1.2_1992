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

/*
  CNC-Programm-Simulator V1.2 BETA DT fuer EMCO-Drehbaenke
  Programm   : Tasha Carl
  Compiler   : MS C6 Model MEDIUM (DATA NEAR/CODE FAR)
  Version    : CGA (nur CNCSIM.C & SIMUL.C) DT
*/


#include "..\CNCSIM.H"

/*=================*/
/*=== PRGMODULE ===*/
/*=================*/

#include "..\optionen.h"
#include "..\Editor.h"
#include "..\Diskrout.h"
#include "..\simul.h"

/*===========================*/
/*=== STEUERPRG PROTOTYPEN ===*/
/*===========================*/

void ProgammInit(void);
void Bildchirmmaske(void);
void Haupmenue(char*,char*);
void EndMske(void);
void DosIit(void);

/*===================================*/
/*=== TYP,DEFINITIONEN,STRUCTUREN ===*/
/*===================================*/


/*=========================*/
/*=== GLOBALE VARIABELN ===*/
/*=========================*/

/*=== Configuration ===*/
struct configstr prgconfig;
struct configstr *config_ptr=&prgconfig;
char VGA=0;

/*=== CNCProgramm-Daten ===*/
struct cncprgstr cncprg[max_line];
char		 prgerr[max_line];
struct cncprgstr *cncprg_ptr=&cncprg[0];
char prgname[51]={"                                                  "},
     autor  [31]={"                              "},
     sektion[31]={"                              "},
     datum  [21]={"                    "},
     bemerk [51]={"                                                  "},
     st_durchmesser[6]={"     "},
     st_laenge[6]={"     "},
     st_innendm[6]={"     "},
     savename[9]={"        "};
/*=== Verwaltungsvariabeln ===*/
char LPTNummer=1,     /* Aktuelle COM & LPT : aktualisiert aus CONFIG */
     COMNummer=1;
char prg_gesichert=1; /* Flag ob aktuelles Prg nicht gesichert ist */
unsigned to1,to2,to3,to4; /* Time-Out-Werte der Schnittstellen */

/*============================*/
/*=== STEUERPRG PROTOTYPEN ===*/
/*============================*/

void ProgrammInit(void);
void Hauptmenue(char*,char*);
void EndMaske(void);
void DosInit(void);
void Info(void);
char Sicherheitsabfrage(void);

/*=====================*/
/*=== HAUPTPROGRAMM ===*/
/*=====================*/

void main(void)
{
  int beg_drive=_getdrive();
  char wahl1,wahl2;

  ProgrammInit();
  LoadConfiguration(config_ptr);
  Bildschirmmaske();

  do {
       Hauptmenue(&wahl1,&wahl2);
       switch(wahl1)
       {
	 case 0:/* Uebertragen */
		Uebertragen(wahl2);
		break;
	 case 1:/* Editor */
		{
		  Editor(cncprg_ptr);
		  Bildschirmmaske();
		}
		break;
	 case 2:/* Simulator */
		CNCSimulation();
		break;
	 case 3:/* Optionen */
		Optionen(wahl2);
		break;
	 case 4:/* Quit - Sicherheitsabfrage */
		if (!prg_gesichert && !Sicherheitsabfrage())
		    wahl1=0;
		break;
	 case 5:/* Infobox */
		Info();
       } /* Balkenwahl */
     } while (wahl1!=4);

  EndMaske();
  DosInit();
  if (!Schreibe_Config(config_ptr))
    printf("ACHTUNG! Aktuelle Konfiguration konnte nicht gesichert werden!\n\n");
  _chdrive(beg_drive);
}


void ProgrammInit(void)
{
  unsigned *to_pt=(unsigned*)0x0000047C;

  if (!_setvideomode(_HRESBW))
      {
	/* Kein CGA */
	printf("\n\nKeine CGA-Karte gefunden!\n");
	printf("(Einstellung der Grafikkarte in der Installation)\n");
	printf("EMCO-CNC-Simulator - Ende\n\n\n");
	exit(1);
      }

  _setvideomode(_TEXTC80);
  SYS_CursorOff();
  DE_Init();
  /*No_Break();*/

  /* Init des Errorfeldes (editor.c) */
  ErrorInit();

  /* Init des Datenfeldes (editor.c)*/
  DatenfeldInit(cncprg_ptr);

  /* TimeOut-Var setzten : Verminderung des Hand-Snake-Effekts von vorhanden */
  to1=*to_pt;
  to2=*(to_pt+2);
  to3=*(to_pt+4);
  to4=*(to_pt+8);
  *(to_pt  )=0;
  *(to_pt+2)=0;
  *(to_pt+4)=0;
  *(to_pt+8)=0;
}

/* ret 0 wenn nicht Aussteigen */
char Sicherheitsabfrage(void)
{
  char ret;

  SYS_PushColor();
  _settextcolor(7);
  _setbkcolor(6);
  TW_Open_Window(20,10,40,4,TW_RAHMEN_D1,TW_TITEL_UNTEN,30,"ACHTUNG");

  _outtext("*** WARNUNG ***\n");
  _outtext("Aktuelles CNC-Programm  nicht gesichert!");
  _outtext("   Wollen Sie das Programm verlassen?");
  ret=SYS_JaNein(14,4,0,0);
  TW_Close_Window();
  SYS_PopColor();

  return ret;
}

void Bildschirmmaske(void)
{
   _clearscreen(_GCLEARSCREEN);
   _settextcolor(7);
   _setbkcolor(0);

   SYS_PushColor();
   _settextcolor(7);
   _setbkcolor(1);
   _settextposition(1,1);
   _outtext("    EMCO-CNC-Simulator V1.2 CGA DT BETA              STS-CNC-Projekt 4.1992     ");
   SYS_PopColor();

   _settextposition(6,1);
   _outtext("*** AKTUELLES CNC-PROGRAMM ***");
   _outtext("\nProgramm  :");
   _outtext(prgname);
   _outtext("\nBemerkung :");
   _outtext(bemerk);
   _outtext("\nAutor     :");
   _outtext(autor);
   _outtext("\nSektion   :");
   _outtext(sektion);
   _outtext("\nDatum     :");
   _outtext(datum);
   _outtext("\nDateiname :");
   _outtext(savename);
}

void Hauptmenue(char *w1,char *w2)
{
   static char pos_uebertr=0,pos_optionen=0;
   long paper=_getbkcolor();
   short pen =_gettextcolor();
   /* definition der PDMenues ins Datasegment */
	  struct PD_parameter uebertr={3,4,0,1,0, 0,1, 6, 24,NULL,NULL},
			      optionen={40,4,0,1,0, 1,1, 5, 26,NULL,NULL};
	  char relpos_ueb[6]={1,1,2,1,2,1},
	       relpos_opt[5]={1,1,1,1,1},
	       array_ueb[6][24]={" Von Diskette laden    "," Auf Diskette sichern  ",
			  " Von EMCO laden        "," Zur EMCO schicken     ",
			  " Zur Datei exportieren "," Von Datei importieren "},
	       array_opt[7][26]={" RS232 Optionen          "," Printer-Optionen        ",
			  " 2. Monitor vorhanden: - "," Rechnergeschwindigkeit  ",
			  " Standart H-Wert         ","test"};
   /* Balkenmenue */
	  struct BM_parameter bm={NULL,10,1,1,11, 0,1,0, 0,1,2, BM_NOKEY,BM_NOKEY,BM_NOKEY};
	  char balken[100]={" HauptmenÅ: | öbertragen |  | Editor |  | Simulation |  | Optionen |  | Quit |     | Info | "};
   /* Kombiniertes Menue */
   struct KM_parameter km;

   km.anz_bm=6;
   km.pd_vorhanden[0]=1;
   km.pd_vorhanden[1]=0;
   km.pd_vorhanden[2]=0;
   km.pd_vorhanden[3]=1;
   km.pd_vorhanden[4]=0;
   km.pd_vorhanden[5]=0;
   km.balken=&bm;
   km.pdmenue[0]=&uebertr;
   km.pdmenue[3]=&optionen;

   /* Parameter uebergeben */
   uebertr.relpos=relpos_ueb;
   uebertr.vorwahl=pos_uebertr;
   uebertr.array =array_ueb[0];
   optionen.relpos=relpos_opt;
   optionen.vorwahl=pos_optionen;
   optionen.array =array_opt[0];
   bm.balken=balken;

   if (MON2_vorhanden)
       array_opt[2][23]='˚';
       else array_opt[2][23]='-';

   /* Hauptmenuewahl */
   _settextcolor(1);
   _setbkcolor(11);
   bm.erste_position=*w1;
   KM_Menue(w1,w2,&km);
   _settextcolor(pen);
   _setbkcolor(paper);
}

void EndMaske(void)
{
   _clearscreen(_GCLEARSCREEN);
   printf("----------------------------------------------------\n");
   printf("\nStÑdtische Technische Schule Eupen\nEMCO-CNC-SoftwareProjekt\n");
   printf("Progr.: CNC-Simulator V1.2 BetaCGA DT (7.1992/Tasha Carl)\n\n\n");
}

void DosInit(void)
{
  unsigned *to_pt=(unsigned*)0x0000047C;

  SYS_CursorOn();

  /* Time-Out-Var. restaurieren */
  *to_pt    =to1;
  *(to_pt+2)=to2;
  *(to_pt+4)=to3;
  *(to_pt+8)=to4;
}

void Info(void)
{
  SYS_PushColor();
  _settextcolor(7);
  _setbkcolor(5);
  TW_Open_Window(20,10,45,8,TW_RAHMEN_D2,TW_TITEL_UNTEN,30,"INFOBOX");

  _outtext("        EMCO-Drehbanksimulator V1.2 DT\n");
  _outtext(" (c) 1992 StÑdtische Technische Schule Eupen\n\n");
  _settextcolor(9);
  _outtext("                  Programm :\n");
  _outtext("                   Tasha Carl");

  getch();

  TW_Close_Window();
  SYS_PopColor();
}
