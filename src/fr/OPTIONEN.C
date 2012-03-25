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

#include "optionen.h"

/* Typen- & Structurdef */
#include "cncsim.h"

extern struct configstr prgconfig;
#define lpt_ 1
#define com_ 0

static struct PD_parameter menue={17,3, 0 , 1,1,0,0,5,0,NULL,NULL};

/* interne Prototypen */
void Wartezeiten(void);
void LPT_waehlen(void);
void COM_waehlen(void);
void Menue_definieren(char);
void H_Wert_eingeben(void);

/* Functionen */

void LoadConfiguration(struct configstr *config)
{
  FILE *configdatei;
  char error,anz;

  do {
       configdatei=fopen("cncsim.cfg","r");
       error=(char)DE_DiskError(configdatei,"Fichier de config.");
     } while (error==DE_REPEAT);

  if (error==DE_NOERROR)
    {
      /* Gefunden - Jetzt reinladen */
      anz=(char)fread(config,sizeof(*config),1,configdatei);
      fclose(configdatei);

      if (anz==0 || strcmp(config->intern1,"‘eeNÚ,d²")) error=DE_ERROR;
    } /* von Laden */

  if (error==DE_ERROR)
    {
      /* Nicht gefunden = neue Erzeugen */
      SYS_PushColor();
      _settextcolor(3);
      _setbkcolor(12);
      TW_Open_Window(10,10,51,4,TW_RAHMEN_D1,TW_TITEL_UNTEN,2,
		    "Configuration");
      _settextcolor(0);
      _outtext(" Erreur au chargement du fichier configuration \n");
      _settextcolor(3);
      _outtext(" Cr‚ation d'un nouveau:\n");
      _outtext("   RS232    : COM 1\n   Imprimante: LPT 1");

      strcpy(config->password,"STS                ");
      for (anz=0;config->password[anz]!=0;anz++)
	   config->password[anz]=(char)(config->password[anz]+1+anz);
      strcpy(config->intern1,"‘eeNÚ,d²");
      strcpy(config->intern2,"-NY7ñ");
      config->monitor2=0;
      config->com=0;
      config->lpt=0; /* 0=LPT1/COM1 ... */
      config->wait_g01=0;
      config->wait_g00=0;
      config->h_norm=100;
      getch();

      if (!Schreibe_Config(config))
       {
	 TW_Close_Window();
	 SYS_PopColor();
	 _clearscreen(_GCLEARSCREEN);
	 printf("Erreur lors de la cr‚ation du fichier de configuration!");
	 printf("CNCSIM Termin‚!");
	 exit(1);
       }

      TW_Close_Window();
      SYS_PopColor();
    } /* von ERROR */
}

void Optionen(char ywahl)
{

   switch(ywahl)
   {
     case 0:{
	      Menue_definieren(com_);
	      COM_waehlen();
	    } break;
     case 1:{
	      Menue_definieren(lpt_);
	      LPT_waehlen();
	    } break;
     case 2:{ /* 2. HGC MON - Nachricht anzeigen */
	      SYS_PushColor();
	      _settextcolor(3);
	      _setbkcolor(12);
	      TW_Open_Window(5,17,67,1,TW_RAHMEN_D1,TW_TITEL_UNTEN,2,"2e ‚cran");
	      _settextcolor(0);

	      if (VGA)
		if (MON2_vorhanden)
		   _outtext(" Une seconde carte graphique a ‚t‚ reconnue.");
		    else _outtext(" Seconde carte graphique absente.");

		else _outtext(" 2. Grafikkarte kann nur mit einer VGA/EGA-Karte betrieben werden!");
	      getch();

	      SYS_PopColor();
	      TW_Close_Window();
	    } break;
     case 3:Wartezeiten(); break;
     case 4:H_Wert_eingeben(); break;
   }
}

void H_Wert_eingeben(void)
{
   struct LE_parameter edit={NULL,"0123456789",19,0,0,0, 0,0,0,0,0,1,0,1};
   char buffer[10],ret;

   SYS_PushColor();
   _settextcolor(3);
   _setbkcolor(12);
   TW_Open_Window(5,17,70,2,TW_RAHMEN_D1,TW_TITEL_UNTEN,2,"valeur H d‚faut");
   _settextcolor(0);

   sprintf(buffer,"%-4d",prgconfig.h_norm);
   _outtext("Pr‚cisez la valeur H par d‚faut:");
   _outtext("valeur d‚faut:");

   SYS_CursorOn();
   edit.y=2;
   edit.editstr=buffer;
   ret=LE_LineEdit(&edit);
   SYS_CursorOff();

   if (ret!=LE_ESC)
      prgconfig.h_norm=atoi(buffer);

   TW_Close_Window();
   SYS_PopColor();
}

void Wartezeiten(void)
{
   char g00[6],g01[6],ret;
   struct LE_parameter edit={NULL,"0123456789",28,0,0,0, 0,0,0,0,0,1,0,1};

   SYS_PushColor();
   _settextcolor(3);
   _setbkcolor(12);
   TW_Open_Window(10,17,59,4,TW_RAHMEN_D1,TW_TITEL_UNTEN,2,"Temps d'attente");
   _settextcolor(0);

   sprintf(g00,"%-4d",prgconfig.wait_g00);
   sprintf(g01,"%-4d",prgconfig.wait_g01);

   _outtext("Temps entre les mouvements pendant la simulation:\n");
   _outtext("\n");
   _outtext("Commande G00              :");
   _outtext(g00);
   _outtext("\nCommandes G01/02/...      :");
   _outtext(g01);
   SYS_CursorOn();

   edit.y=3;
   edit.editstr=g00;
   ret=LE_LineEdit(&edit);

   edit.editstr=g01;
   edit.y=4;
   if (ret!=LE_ESC)
      ret=LE_LineEdit(&edit);

   SYS_CursorOff();
   if (ret!=LE_ESC)
    {
      prgconfig.wait_g00=atoi(g00);
      prgconfig.wait_g01=atoi(g01);
    }

   TW_Close_Window();
   SYS_PopColor();
}

char Schreibe_Config(struct configstr *config) /* ret OK=1 */
{
   FILE *configdatei;
   char ret=1,error;

   do {
	configdatei=fopen("cncsim.cfg","w");
	error=(char)DE_DiskError(configdatei,"Fichier config.");
      } while (error==DE_REPEAT);

   if (error==DE_ERROR) ret=0;

   if (fwrite(config,sizeof(*config),1,configdatei)!=1) ret=0;
   fclose(configdatei);

   return ret;
}

void COM_waehlen(void)
{
   char ret;

   SYS_PushColor();
   _settextcolor(3);
   _setbkcolor(12);
   TW_Open_Window(10,10,47,10,TW_RAHMEN_D1,TW_TITEL_OBEN,2,"Port s‚rie");
   _settextcolor(0);

   _outtext(" Choisissez le port s‚rie:\n\n\n\n\n\n\n\n");
   _outtext("  *: port s‚rie inn‚xistant.\n");

   menue.vorwahl=prgconfig.com;
   ret=PD_Menue(&menue);
   if (ret!=PD_RET_ESC && ret!=4)
      prgconfig.com=ret;

   TW_Close_Window();
   SYS_PopColor();
}

void LPT_waehlen(void)
{
   char ret;

   SYS_PushColor();
   _settextcolor(3);
   _setbkcolor(12);
   TW_Open_Window(10,10,45,10,TW_RAHMEN_D1,TW_TITEL_OBEN,2,"Port parallŠle");
   _settextcolor(0);


   _outtext(" Choisissez le port parallŠle:\n\n\n\n\n\n\n\n");
   _outtext("  *: port parallŠle inn‚xistant.\n");

   menue.vorwahl=prgconfig.lpt;
   ret=PD_Menue(&menue);
   if (ret!=PD_RET_ESC && ret!=4)
      prgconfig.lpt=ret;

   TW_Close_Window();
   SYS_PopColor();
}

void Menue_definieren(char flag)
{
  char z ;
  /* Nicht auf den Stack!! */
  static char relpos[5]={1,1,1,1,1};
  static char array1[5][12]={"   LPT 1   ","   LPT 2   ","   LPT 3   ","   LPT 4   ","   Fin.    "};
  static char array2[5][12]={"   COM 1   ","   COM 2   ","   COM 3   ","   COM 4   ","   Fin.    "};

  menue.xap=17;
  menue.anz=5;
  menue.strbr=12;
  menue.relpos=relpos;
  menue.escerlaubt=1;

  if (flag==lpt_)
    {
      /* Test ob vorhanden */
      for (z=0;z<4;z++)
	{
	  array1[z][9]=' ';
	  if (!SYS_ExistLPT(z)) array1[z][9]='*';
	  array1[z][1]=' ';
	}
      menue.array=array1[0];
      array1[prgconfig.lpt][1]='û';
    }
   else {
	   /* Test ob vorhanden */
	   for (z=0;z<4;z++)
	    {
	      array2[z][9]=' ';
	      if (!SYS_ExistCOM(z)) array2[z][9]='*';
	      array2[z][1]=' ';
	    }
	   menue.array=array2[0];
	   array2[prgconfig.com][1]='û';
	}
}
