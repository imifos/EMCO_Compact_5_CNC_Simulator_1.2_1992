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
  CNC-Programm-Simulator V1.2 BETA VGA fuer EMCO-Drehbaenke
  Programm   : Tasha Carl
  Compiler   : MS C6 Model MEDIUM (DATA NEAR/CODE FAR)
  Version    : VGA Franz”sisch
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

/*=========================*/
/*=== GLOBALE VARIABELN ===*/
/*=========================*/

/*=== Configuration ===*/
struct configstr prgconfig;
struct configstr *config_ptr=&prgconfig;
char VGA=1;

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
    printf("ATTENTION! Impossible de sauvgarder la configuration!\n\n");
  _chdrive(beg_drive);
}


void ProgrammInit(void)
{
  unsigned *to_pt=(unsigned*)0x0000047C;

  if (!_setvideomode(_VRES16COLOR))
      {
	/* Kein VGA */
	printf("\n\nCarte VGA absente!\n");
	printf("Simulateur-EMCO-CNC - Termin‚\n\n\n");
	exit(1);
      }

  _setvideomode(_TEXTC80);
  SYS_CursorOff();

  DE_Init();
  No_Break();


  /* Init des Errorfeldes (editor.c) */
  ErrorInit();

  /* Init des Datenfeldes (editor.c)*/
  DatenfeldInit(cncprg_ptr);

  /* TimeOut-Var setzten : Verminderung des Hand-Shake-Effekts von vorhanden */
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
  TW_Open_Window(20,10,40,4,TW_RAHMEN_D1,TW_TITEL_UNTEN,25,"ATTENTION");

  _outtext("*** ATTENTION ***\n");
  _outtext("   Programme courant n'est pas sauv‚!\n");
  _outtext("   Voulez-vous quitter  le programme? ");
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
   _outtext("  Simulateur EMCO - CNC V1.2 FR VGA Beta          Projet CNC -- STS Eupen 4.92  ");
   SYS_PopColor();

   _settextposition(6,1);
   _outtext("*** PROGRAMME COURANT ***");
   _outtext("\nProgramme     :");
   _outtext(prgname);
   _outtext("\nCommentaire   :");
   _outtext(bemerk);
   _outtext("\nAuteur        :");
   _outtext(autor);
   _outtext("\nSection       :");
   _outtext(sektion);
   _outtext("\nDate          :");
   _outtext(datum);
   _outtext("\nNom du fichier:");
   _outtext(savename);

   /* 2. HGC-Monitor */
   if (MON2_Init())
       {
	 MON2_clrscr(1,1,80,25);
	 MON2_settextposition(1,1);
	 MON2_outtext("Simulateur CNC-EMCO");
	 MON2_settextposition(1,3);
	 MON2_outtext("(c) 1992 by STŽDTISCHE TECHNISCHE SCHULE EUPEN");
	 MON2_settextposition(1,4);
	 MON2_outtext("programm‚ par Tasha Carl (Made in Belgium/Eupen)");
       }
}

void Hauptmenue(char *w1,char *w2)
{
   static char pos_uebertr=0,pos_optionen=0;
   long paper=_getbkcolor();
   short pen =_gettextcolor();
   /* definition der PDMenues ins Datasegment */
   static struct PD_parameter uebertr={3,4,0,1,0, 0,1, 4, 24,NULL,NULL},
			      optionen={40,4,0,1,0, 1,1, 5, 26,NULL,NULL};
   static char relpos_ueb[6]={1,1,2,1,2,1},
	       relpos_opt[5]={1,1,1,1,1},
	       array_ueb[4][24]={" Charger un programme  "," Sauver un programme   ",
			  " Chargement de l'EMCO  "," Envoyer vers l'EMCO   "},
	       array_opt[7][26]={" Options RS232 (COM)     "," Options d'imprimante    ",
				 " 2. carte graphique  : - "," Vitesse de simulation   ",
				 " Valeur H normalis‚e     "};
   /* Balkenmenue */
   static struct BM_parameter bm={NULL,10,1,1,11, 0,1,0, 0,1,2, BM_NOKEY,BM_NOKEY,BM_NOKEY};
   static char balken[100]={" Choix: | Transmission |  | Editeur |  | Simulation |  | Options |    | Quit |     | Info | "};
   /* Kombiniertes Menue */
   static struct KM_parameter km;

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
       array_opt[2][23]='û';
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
   printf("St„dtische Technische Schule Eupen\nEMCO-CNC-SoftwareProjekt\n");
   printf("Progr.: CNC-Simulator V1.2 BetaVGA FR (7.1992/N.Carl)\n\n\n");

   /* Loeschen wenn vorhanden */
   if (MON2_vorhanden)
	 MON2_clrscr(1,1,80,25);
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
  TW_Open_Window(20,10,45,8,TW_RAHMEN_D2,TW_TITEL_UNTEN,10,"Made in Belgium");

  _outtext("       Simulateur de tour d'EMCO V1.2\n");
  _outtext(" (c) 1992 St„dtische Technische Schule Eupen\n\n");
  _settextcolor(9);
  _outtext("                 Projet CNC:\n");
  _outtext("                programm‚ par \n");
  _outtext("               Tasha Carl");

  getch();

  TW_Close_Window();
  SYS_PopColor();
}
