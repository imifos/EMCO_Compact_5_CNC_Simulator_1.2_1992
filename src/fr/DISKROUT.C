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


#include "diskrout.h"
#include "editor.h"
#include "cncsim.h"

/* externe Var */
extern char prg_gesichert;
extern char prgname[51],
	    autor[31],
	    sektion[31],
	    datum[21],
	    bemerk[51],
	    st_durchmesser[6],
	    st_laenge[6],
	    st_innendm[6],
	    savename[9];
extern struct cncprgstr cncprg[max_line];
extern char		prgerr[max_line];
extern struct configstr	prgconfig;

void (_interrupt _far *oldbreak)(void);
void  _interrupt _far newbreak(void);

/* interne Prototypen */
void Disc_Mem(void);
void Mem_Disc(void);
void Mem_EMCO(void);
void EMCO_Mem(void);
char Pruefe_GMBefehl(char*,char*,char*,char*,char*);
char Test_Zeile(short);
void Analyse_Puffer(short,char*);
int  String_ComPort(int,char*,char);

/* Funktionen */
void Uebertragen(char ywahl)
{
  int dr=_getdrive();
  switch(ywahl)
  {
    case 0:Disc_Mem(); break;
    case 1:Mem_Disc(); break;
    case 3:Mem_EMCO(); break;
    case 2:EMCO_Mem(); break;
  }
  _chdrive(dr);
}

/************************************************************************/
void Disc_Mem(void)
{
   char pfad[_MAX_PATH],name[15],complett[100],
	laden=0,ok=1,laufw,de_flag,janein,flag=0,
	buf[_MAX_PATH],
	bprgname[51],bautor[31],bsektion[31],bdatum[21],bbemerk[51];

   FILE *datei;
   short read_anz,za;

   SYS_PushColor();
   _settextcolor(3);
   _setbkcolor(9);
   TW_Open_Window(15,6,65,12,TW_RAHMEN_S1,TW_TITEL_OBEN,5,"Charger un programme ...");

   while (ok && !laden)
     {
	_clearscreen(_GWINDOW);
	_outtext("\nChoisissez le programme s.v.p.:");
	_outtext("\n(Affichage des infos aves ENTER)");
	_outtext("\nNom du fichier: ");
	_outtext(savename);
	_outtext(".CNC");

	if (!prg_gesichert)
	    {
	       _settextcolor(2);
	       _outtext("\n\a*** ATTENTION ! Le programme actuel n'est pas sauvÇ! ***");
	       _settextcolor(3);
	    }

	if ((ok=DIR_Wahl(22,13,"*.cnc", name,pfad,&laufw))==1)
	  {
	    /* Kompletter Filename basteln */
	    strcpy(complett,pfad);
	    if (strlen(pfad)>4)
	      strcat(complett,"\\");/* Nicht wenn im Root */
	    strcat(complett,name);

	    /*** Programm einblenden ***/
	    _clearscreen(_GWINDOW);
	    _outtext("programme choissi:");

	    /* File oeffnen */
	    do {
		 datei=fopen(complett,"r");
		 de_flag=DE_DiskError(datei,name);
	       } while (de_flag==DE_REPEAT);
	    if (de_flag==DE_ERROR) ok=0;

	    /* Controllcodes testen */
	    if (ok) {
		      read_anz=fread(buf,11,1,datei);
		      if (read_anz==0 || strcmp(buf,controll))
			{
			  _outtext("\n\nPas de fichier CNC!");
			  ok=0;
			  break;
			}
		    }

	    if (ok)
	      {
		/* Infos laden */
		fread(bprgname,51,1,datei);
		fread(bautor,31,1,datei);
		fread(bsektion,31,1,datei);
		fread(bdatum,21,1,datei);
		read_anz=fread(bbemerk,51,1,datei);
		if (read_anz==0) { _outtext("\n\nErreur dans le fichier!");
				   ok=0; }
		  else {
			 _outtext("\n\nProgramme  :");
			 _outtext(bprgname);
			 _outtext("\nAuteur     :");
			 _outtext(bautor);
			 _outtext("\nSection    :");
			 _outtext(bsektion);
			 _outtext("\nDate       :");
			 _outtext(bdatum);
			 _outtext("\nCommentaire:");
			 _outtext(bbemerk);
			 janein=SYS_JaNein(1,9,1,SYS_ESC);
			 if (janein==SYS_JA)   { laden=1; ok=1; }
			 if (janein==SYS_NEIN) { laden=0; ok=1; }
			 if (janein==SYS_ESC)  { _outtext("\n\nESC!"); ok=0; }
		       }
	      }

	  } /* von if ok... */
	  else _outtext("\n\n\n\nInterruption!");

     } /* von while ok & !laden */

   if (!ok)
     { _outtext("\nInterruption du chargement!"); getch(); }

   if (laden)
     {
       _clearscreen(_GWINDOW);
       ErrorInit(); /*Errorfeld init*/
       _outtext("charge programme:\n");
       _outtext(bprgname);

       /* Neu Oeffnen */
       do {
	    fcloseall();
	    datei=fopen(complett,"r");
	    de_flag=DE_DiskError(datei,name);
	  } while (de_flag==DE_REPEAT);
       if (de_flag==DE_ERROR) {
				ok=0;
				_outtext("\n\nErreur pendant le chargement!\n");
			      }
	else
	  {
	    /* Einlesen der Infos */
	    fread(buf,11,1,datei); /* Controllstr */
	    fread(prgname,51,1,datei);
	    fread(autor,31,1,datei);
	    fread(sektion,31,1,datei);
	    fread(datum,21,1,datei);
	    fread(bemerk,51,1,datei);
	    fread(st_durchmesser,6,1,datei);
	    fread(st_laenge,6,1,datei);
	    fread(st_innendm,6,1,datei);

	    /* Name zerlegen */
	    strcpy(savename,name);
	    for (za=0;savename[za]!=0;za++)
		if (savename[za]=='.') savename[za]=0;
	    while (strlen(savename)<8)
		strcat(savename," ");

	    /* Daten einlesen */
	    for (za=0;za<max_line;za++)
	      {
		do read_anz=fread(&cncprg[za],sizeof(cncprg[0]),1,datei);
		    while ((flag=DE_DiskError(datei,name))==DE_REPEAT);
		if (flag!=DE_NOERROR) break;
		if (read_anz==0) {
				   _outtext("\n\n\nErreur dans le fichier!");
				   flag=DE_ERROR; break;
				 }
		if (kbhit()) if (getch()==27)
		    {
		      _outtext("\n\n\nInterruption avec ESC");
		      flag=DE_ERROR; break;
		    }
	      } /* von ladeschleife za */
	    fclose(datei);

	    if (flag==DE_ERROR)
	     {
	       _outtext("\n\nErreur apparru!");
	       printf("\a");
	       /* Im Falle eines Errors Speicher loeschen */
	       DatenfeldInit(&cncprg[0]);
	       /* Fehlt: ProgrInfo reset */
	     }
	     else {
		    _outtext("\n\nOK, programme est chargÇ!");
		    prg_gesichert=1;
		  }
	  } /* von else neu oeffnen */

       getch();
     } /* von if (laden).. */

   TW_Close_Window();
   SYS_PopColor();
   Bildschirmmaske();
}

/************************************************************************/
void Mem_Disc(void)
{
   char pfad[_MAX_PATH]={"  "},
	complett[100],
	sichern=0,raus,flag,zx,zy,esc=0,ret,
	laufw_st[3]={" "},
	*leer="                                                      ",
	*menge1="abcdefghijklmnopqrstuvwxyz1234567890\\_-:.",
	*menge2="abcdefghijklmnopqrstuvwxyz1234567890_-",
	menge3[20],
	filename[10];
   struct LE_parameter para={NULL,NULL,20,0,0,0,0,0,0,0,0,1,0,0};
   short za,aktdr,drive,write_anz;
   struct DIR_rec dir;
   FILE *datei;

   TW_Open_Window(2,3,78,21,TW_RAHMEN_S1,TW_TITEL_OBEN,5,"Sauver le programme...");
   strcpy(filename,savename);

   /* Laufwerksmenge herstellen */
   /* Wenn laufwerk anwaehlbar, dann existiert es */
   aktdr=_getdrive();
   for(drive=1,zx=0;drive<27;drive++)
	if(!_chdrive(drive))
	  {
	    menge3[zx++]=(char)(drive+'A'-1);
	    menge3[zx]=0;
	  }
   _chdrive(aktdr);
   laufw_st[0]=(char)(aktdr+'A'-1);

   while (!sichern && !esc)
    {
       /* aktueller Pfad feststellen */
       getcwd(pfad,_MAX_PATH);
       while (strlen(pfad)<65)
	 strcat(pfad," ");

       /* Laufwerk feststellen */
       aktdr=_getdrive();
       laufw_st[0]=(char)(aktdr+'A'-1);

       /* Dir einlesen & ausgeben */
       strcpy(complett,pfad);
       if (strlen(pfad)>4)
	  strcat(complett,"\\*.CNC");
	  else strcat(complett,"*.CNC"); /* Nicht wenn im Root */
       DIR_Make("*.CNC",_A_NORMAL);
       DE_ResetErrorMemory();

       _clearscreen(_GWINDOW);
       for (za=1,zx=0,zy=1;za<=DIR_eintraege;za++)
	{
	  DIR_ReadNr(za,&dir);
	  _settextposition(zy,zx*15+3);
	  _outtext(dir.name);
	  zx++;
	  if (zx==5) {
		       zx=0;
		       zy++;
		     }
	}


       /* Eingaben machen */
       _settextposition(19,14);
       _outtext(leer);
       _settextposition(20,14);
       _outtext(leer);
       _settextposition(21,14);
       _outtext(leer);
       para.esc_erlaubt=1;

       raus=0;
       do {
	    _settextposition(19,1);
	    _outtext("Lecteur    :");
	    _outtext(laufw_st);
	    _settextposition(20,1);
	    _outtext("Chemin     :");
	    _outtext(pfad);
	    _settextposition(21,1);
	    _outtext("Fichier    :");
	    SYS_CursorOn();

	    para.raus_oben=1;
	    para.x=13;
	    para.y=21;
	    para.editstr=filename;
	    para.zeichenmenge=menge2;
	    ret=LE_LineEdit(&para);
	    if (ret==LE_OK)
		{ /* Name ist eingegeben */
		  /* Test ob vorhenden */
		  strcpy(complett,SYS_DelEndSpaces(pfad));
		  if (strlen(pfad)>4)
		      strcat(complett,"\\"); /* Nicht wenn im Root */
		  strcat(complett,SYS_DelEndSpaces(filename));
		  strcat(complett,".CNC");
		  SYS_CursorOff();
		  if ((flag=DE_FileExit(complett))==DE_NOEXIST)
		    {
		      raus=1;  sichern=1;
		    }
		    else if (flag==DE_ESC) {
					     raus=1; esc=1;
					   }
		  SYS_CursorOn();
		}
	    if (ret==LE_OBEN_RAUS)
		{
		  /* Eingabe eines neuen Pfades */
		  para.raus_oben=1;
		  para.x=13;
		  para.y=20;
		  para.editstr=pfad;
		  para.zeichenmenge=menge1;
		  ret=LE_LineEdit(&para);
		  if (ret==LE_OK)
		    {
		      raus=1;  chdir(pfad);
		    }
		  if (ret==LE_OBEN_RAUS)
		    { /* Laufwerk */
		      para.raus_oben=0;
		      para.x=13;
		      para.y=19;
		      para.editstr=laufw_st;
		      para.zeichenmenge=menge3;
		      ret=LE_LineEdit(&para);
		      if (ret==LE_OK) {
					aktdr=_getdrive();
					do {
					     _chdrive((int)laufw_st[0]-'A'+1);
					     datei=fopen("tmp.cnc","r");
					     fclose(datei);
					   } while ((flag=DE_DiskError((FILE*)1," Laufwerk"))==DE_REPEAT);
					if (flag!=DE_NOERROR)
					      _chdrive(aktdr);
					raus=1;
				      }
		    }
		}
	     SYS_CursorOff();
	     if (ret==LE_ESC) {
				raus=1;
				esc=1;
			      }
	  } while (!raus); /* von Eingabe-while */
    } /* von Dir-while */

   /* Sichern */
   _settextposition(19,1);
   _outtext(leer);
   _settextposition(20,1);
   _outtext(leer);
   _settextposition(21,1);
   _outtext(leer);
   if (!esc)
    {
       Programmdaten_eingeben();
       do datei=fopen(complett,"w");
	  while ((flag=DE_DiskError(datei,filename))==DE_REPEAT);
       if (flag==DE_NOERROR)
	 {
	    _settextposition(20,1);
	    _outtext("Ecriture...");
	    fwrite(controll,11,1,datei);
	    fwrite(prgname,51,1,datei);
	    fwrite(autor,31,1,datei);
	    fwrite(sektion,31,1,datei);
	    fwrite(datum,21,1,datei);
	    fwrite(bemerk,51,1,datei);
	    fwrite(st_durchmesser,6,1,datei);
	    fwrite(st_laenge,6,1,datei);
	    fwrite(st_innendm,6,1,datei);

	    strcpy(savename,filename);
	    _settextposition(21,1);
	    for (za=0;za<max_line;za++)
		{
		  do write_anz=fwrite(&cncprg[za],sizeof(cncprg[0]),1,datei);
		     while ((flag=DE_DiskError(datei,filename))==DE_REPEAT);

		  if (flag!=DE_NOERROR) break;

		  if (write_anz==0) {
				      _outtext("Pas de place sur la disquette - ");
				      flag=DE_ERROR; break;
				    }
		  if (kbhit()) if (getch()==27) {
						  _outtext("Interruption avec ESC -");
						  flag=DE_ERROR; break;
						}
		}
	    fcloseall();
	    if (flag==DE_ERROR)
		{ _outtext("Erreur d'Çcriture!"); printf("\a"); }
		else {
		       _outtext("OK, Programme sauvÇ !");
		       prg_gesichert=1;
		     }
	    getch();
	 }
    }

   /* Ende */
   TW_Close_Window();
}

/* Syntaxueberpruefung des Programms */
char Syntax(void)
{
   short bis_zeile,
	 zeile,esc=0;
   char buf[80],error=0;

   ErrorInit();
   /* letzte Zeile suchen */
   for (zeile=max_line-1;zeile>0 && !strcmp(cncprg[zeile].GM,"    ");zeile--)
     bis_zeile=zeile;

   /* Raus wenn kein Programm im Speicher */
   if (bis_zeile==1) return 255;

   /* Fenster oeffnen & maske */
   SYS_PushColor();
   _settextcolor(7);
   _setbkcolor(1);
   TW_Open_Window(15,10,30,4,TW_RAHMEN_D2,TW_TITEL_OBEN,3,"Test de syntax");

   _outtext("\n Traitement ligne     de  ");
   sprintf(buf,"%03d",bis_zeile-1);
   _outtext(buf);

   for (zeile=0;zeile<bis_zeile && !esc;zeile++)
    {
      sprintf(buf,"%03d",zeile);
      _settextposition(2,19);
      _outtext(buf);
      if (Test_Zeile(zeile))
	{
	  _settextposition(3,2);
	  _outtext("Derniäre faute ds ligne ");
	  sprintf(buf,"%03d ",zeile);
	  _outtext(buf);
	  error=1;
	}
      if (kbhit()) esc=(getch()==27) ? 1:0;
    }

   if (error) {
		_settextposition(4,2);
		_outtext("Erreurs  dans  le  programm!");
		getch();
	      }

   TW_Close_Window();
   SYS_PopColor();

  return error;
}


/* testet eine Zeile und setzt das CNCERR-Feld
   Bit 0: unbekannter G/M-Befehl
       1: Zuviele Parameter
       2: Zuwenig Parameter
       3: I oder K erwartet (X/Z)
       4: Standart H-Befehl wurde gesetzt
       5: FLKT-Error
   return 1 wenn Fehler und 0 wenn nicht
*/
char Test_Zeile(short zeile)
{
   #define error   1
   #define noerror 0
   char xi,zk,flkt,h; /* Returnwerte der Datenbankfunktion */
   char ret=0;

   /* Informationen holen */
   if (Pruefe_GMBefehl(cncprg[zeile].GM, /*Raus:*/ &xi,&zk,&flkt,&h))
    { /* Unbekannter Befehl */
      prgerr[zeile]=1;	/* Bit 1 setzten */
      return error;
    }
    else
    {
      /* Befehl bekannt, jetzt check der verschiedenen Felder */

      /* XI */
      switch (xi)
      {
	case 'O':if (strcmp(cncprg[zeile].XI,"      "))	/* XI muss leer sein, wennn nicht */
		   {prgerr[zeile]|=2; ret=error;}	/* setzte bit 1 */
		 break;
	case 'X':if (cncprg[zeile].XI[0]!=' ' && cncprg[zeile].XI[0]!='-') /* nur X-Wert erlaubt */
		    {prgerr[zeile]|=64; ret=error;}			   /* sonst Bit 6 */
		 break;
	case 'I':if (cncprg[zeile].XI[0]!='I')				   /* nur I-Wert sonst Bit 3 */
		    {prgerr[zeile]|=8; ret=error;}
      }

      /* ZK */
      switch (zk)
      {
	case 'O':if (strcmp(cncprg[zeile].ZK,"      "))
		    {prgerr[zeile]|=2; ret=error;}
		 break;
	case 'Z':if (cncprg[zeile].ZK[0]!=' ' && cncprg[zeile].ZK[0]!='-')
		    {prgerr[zeile]|=64; ret=error;}
		 break;
	case 'K':if (cncprg[zeile].ZK[0]!='K')
		    {prgerr[zeile]|=8; ret=error;}
      }

      /* FLKT */
      switch (flkt)
      {
	case 'O':if (strcmp(cncprg[zeile].FLKT,"    "))
		    {prgerr[zeile]|=2; ret=error;}
		 break;
	case 'F':if (cncprg[zeile].FLKT[0]!=' ' && cncprg[zeile].ZK[0]!='-')
		    {prgerr[zeile]|=32; ret=error;}
		 break;
	case 'L':if (cncprg[zeile].FLKT[0]!='L')
		    {prgerr[zeile]|=128; ret=error;}
		 break;
	case 'K':if (cncprg[zeile].FLKT[0]!='K')
		    {prgerr[zeile]|=128; ret=error;}
		 break;
	case 'T':if (cncprg[zeile].FLKT[0]!='T')
		    {prgerr[zeile]|=128; ret=error;}
      }

      /* H */
      switch(h)
      {
	case 'O':if (strcmp(cncprg[zeile].H,"    "))
		    {prgerr[zeile]|=128; ret=error;}
		 break;
	case 'H':if (!strcmp(cncprg[zeile].H,"    "))  /* wenn H leer dann Standartwert */
		    {
		      prgerr[zeile]|=16;
		      ret=error;
		      sprintf(cncprg[zeile].H,"%3d ",prgconfig.h_norm);
		    }
      }

    } /* von Befehl bekannt -> check */
  return ret;
}

/* Datenbank der G & M Funktionen:
   Rein: Zeiger auf G/M-Feld
   Ret : Entsprechenden Parametercodes (' ' wenn leer)
	 Returnwert =1 wenn G/M unbekannt
 */
char Pruefe_GMBefehl(char *in_gm, char *xi,char *zk,char *flkt,char *h)
{
   #define anz_G 24
   #define anz_M 13
   #define unbekannt 1
   #define bekannt   0
   struct para_feld {
		      char GM;
		      char XI;
		      char ZK;
		      char FLKT;
		      char H;
		      char text[50];
		    };
   char gm, za;

   /* STATIC damit die Daten nicht im Stack abgelegt werden!! */
   static struct para_feld Gdatenbank[anz_G]={
     {0 ,'X','Z','O','O',"Eilgang"},
     {1 ,'X','Z','F','O',"Geraden-Interpolation"},
     {2 ,'X','Z','F','O',"Kreis-Interpolation (im Uhrzeigersinn)"},
     {3 ,'X','Z','F','O',"Kreis-Interpolation (in Gegenuhrzeigersimm)"},
     {4 ,'X','O','O','O',"Verweilzeit"},
     {21,'O','O','O','O',"Leerzeile/Bemerkung"},
     {24,'O','O','O','O',"Nicht benutzt"},
     {25,'O','O','L','O',"Unterprogrammaufruf"},
     {26,'X','Z','T','O',"Werkzeugwechsel"},
     {27,'O','O','L','O',"Sprungbefehl"},
     {33,'O','Z','K','O',"Gewinde mit gleichbleibender Steigung"},
     {73,'O','Z','F','O',"Spanbruchzyklus"},
     {78,'X','Z','K','H',"Gewindezyklus"},
     {81,'O','Z','F','O',"Bohrzyklus"},
     {82,'O','Z','F','O',"Bohrzyklus mit Verweilzeit"},
     {83,'O','Z','F','O',"Aushebbohrzyklus"},
     {84,'X','Z','F','H',"LÑngsdrehzyklus"},
     {85,'O','Z','F','O',"Ausreibzyklus"},
     {86,'X','Z','F','H',"Einstechzyklus"},
     {88,'X','Z','F','H',"Plandrehzyklus"},
     {89,'O','Z','F','O',"Ausreibzyklus mit Verweilzeit"},
     {90,'O','O','O','O',"Absolutwertkoordinaten einschalten"},
     {91,'O','O','O','O',"Inkrementalkoordinaten einschalten"},
     {92,'X','Z','O','O',"Speichernullpunkt setzten"}
				    };

   static struct para_feld Mdatenbank[anz_M]={
     {0 ,'O','O','O','O',"Programmierter Halt"},
     {3 ,'O','O','O','O',"Spindel im Uhrzeigersinn"},
     {5 ,'O','O','O','O',"Spindel halt"},
     {6 ,'X','Z','T','O',"Werkzeugwechsel"},
     {17,'O','O','O','O',"RÅcksprungbefehl"},
     {30,'O','O','O','O',"Programmende"},
     {99,'I','K','O','O',"Kreisparameter"},
     {98,'X','Z','O','O',"Automatischer Spielausgleich"},
     {8 ,'O','O','O','O',"X62 Pin 15 High"},
     {9 ,'O','O','O','O',"X62 Pin 15 Low"},
     {22,'O','O','O','O',"X62 Pin 18 High"},
     {23,'O','O','O','O',"X62 Pin 18 Low"},
     {26,'O','O','O','H',"Impulse auf X62 Pin 20"}
					  };

  gm=(char)atoi(in_gm+1);
  if (in_gm[0]!='M')
    {
      za=0;
      while (Gdatenbank[za].GM!=gm && za<anz_G) za++;
      if (za==anz_G) return unbekannt; /* Befehl nicht gefunden -> raus */

      /* Gefunden */
      *xi  =Gdatenbank[za].XI;
      *zk  =Gdatenbank[za].ZK;
      *flkt=Gdatenbank[za].FLKT;
      *h   =Gdatenbank[za].H;
    }
    else
       {
	 za=0;
	 while (Mdatenbank[za].GM!=gm && za<anz_M) za++;
	 if (za==anz_M) return unbekannt; /* Befehl nicht gefunden -> raus */

	 /* Gefunden */
	 *xi  =Mdatenbank[za].XI;
	 *zk  =Mdatenbank[za].ZK;
	 *flkt=Mdatenbank[za].FLKT;
	 *h   =Mdatenbank[za].H;
       }
   return bekannt;
}

/* EMCO-Programm Åber die serielle Schnittstelle in den Puffer laden */
void EMCO_Mem(void)
{
   static char combuf[7000];	/* Puffer beim Einlesen von COM: */
   int esc=0,weiter=0,zeile,comza;
   char wert,oldwert,punktzaehler,buf[10];

   /* Ende wenn COM nicht vorhanden */
   if (!SYS_ExistCOM(prgconfig.com))
    {
      SYS_PushColor();
      _settextcolor(3);
      _setbkcolor(9);
      TW_Open_Window(25,10,51,3,TW_RAHMEN_S1,TW_TITEL_OBEN,5,"Erreur");
      _outtext(" Erreur:\n Interface sÇrie configurÇe n'existe pas!\n (Touche s.v.p.)");
      getch();
      TW_Close_Window();
      SYS_PopColor();
      return;
    }

   SYS_PushColor();
   _settextcolor(3);
   _setbkcolor(9);
   TW_Open_Window(25,10,45,8,TW_RAHMEN_S1,TW_TITEL_OBEN,5,"EMCO -> COM:");

   /* Maske */
   _outtext("Chargement d'un programme EMCO  par  le  portsÇrie.");
   _outtext("\n\nEtat  :\nLigne :ID\n\n\n          (Interruption avec ESC)");
   _settextposition(6,8);  _outtext(">");
   _settextposition(6,39); _outtext("<");

   /* Initialisierung der Schnittstelle */
   SER_Setzte_Parameter(prgconfig.com,DATABITS_7+STOPBIT_1+PARIT_EVEN+BAUD_300);

   /* Warte auf Daten */
   _settextcolor(5);
   _settextposition(4,8);
   _outtext("J'attend sur un programme");
   do {
	if (kbhit()) esc=getch();		  /* eventuell ESC */
	if (SER_Zeichen_wartet(prgconfig.com))	  /* wenn Zeichen, dann Test ob Startzeichen % */
	    {
	      if (SER_Lese_Zeichen(prgconfig.com,&wert)!=SER_OK) esc=27;
	      if (wert=='%') weiter=1;
	    }
      } while (esc!=27 && !weiter);

   if (esc!=27)
    {
      /* CR hinter Startzeichen '%' laden */
      while (!SER_Zeichen_wartet(prgconfig.com));
      SER_Lese_Zeichen(prgconfig.com,&wert);

      /* LF hinter Startzeichen '%' laden */
      while (!SER_Zeichen_wartet(prgconfig.com));
      SER_Lese_Zeichen(prgconfig.com,&wert);

      _settextposition(4,8);
      _outtext("Je reáois un programme...");
      _settextcolor(3);
      punktzaehler=0;
      comza=0;
      oldwert=0;
      weiter=0;
      zeile=-1;

      do {
	   /* Warten bis neues Zeichen anliegt */
	   do if (kbhit()) esc=getch();
	     while (esc!=27 && !SER_Zeichen_wartet(prgconfig.com));

	   /* lesen & speichern */
	   oldwert=wert;
	   if (esc!=27)
	     if (SER_Lese_Zeichen(prgconfig.com,&wert)!=SER_OK) esc=27;
	   combuf[comza++]=wert;

	   /* Arbeitsvorgang visualisieren */
	   if (wert!=0x0a) punktzaehler++;
	   if (wert==0x0d) {
			      zeile++;
			      _settextposition(5,8);
			      sprintf(buf,"%03d ",zeile);
			      _outtext(buf);
			      punktzaehler=0;
			      _settextposition(6,9);
			      _outtext("                              ");
			   }
	   if (wert!=0x0a && wert!=0x0d) {
					   _settextposition(6,8+punktzaehler);
					   _outtext(".");
					 }

	   /* Ende der Uebertragung:  ' M' */
	   if (oldwert==' ' && wert=='M') weiter=1;

	 } while (!weiter && esc!=27);/* von Prgladeschleife */

       /* Bildschirmausgabe korrigieren */
       _settextposition(6,9);
       _outtext("                              ");
       zeile--;
       _settextposition(5,8);
       sprintf(buf,"%03d ",zeile);
       _outtext(buf);

    } /* von Laden wenn ! ESC */

    if (esc!=27)
     {
       /* Puffer in Programm umwandeln */
       _settextposition(4,8);			/* Bildschirm */
       _settextcolor(5);
       _outtext("Analyse des informations...");
       DatenfeldInit(&cncprg[0]);		/* altes Prg loeschen */
       Analyse_Puffer(zeile,combuf);		/* Analysieren	      */

       _settextposition(4,8);			/* Formatieren */
       _outtext("Formatage du programme...  ");
       Format();

       _settextposition(4,8);			/* Syntax */
       _outtext("Verification du syntaxe... ");
       Syntax();
       _settextcolor(3);
     }

   /* Alles beendet */
   _settextposition(8,11);
   if (esc==27) _outtext("  Interruption avec ESC   ");
      else _outtext("   Transmission finie!   ");
   getch();
   SYS_PopColor();
   TW_Close_Window();
}

void Analyse_Puffer(short zeilen,char *bufadr)
{
  struct bufline {
		   char dummy1[3],   /* Man "lege" eine Structur "ueber" */
			N     [3],   /* den Puffer, und kann so auf alle */
			GM    [3],   /* Elemente bequem zurueckgreifen.  */
			XI    [6],
			dummy2[1],
			ZK    [6],
			FLKT  [4],
			H     [4],
			CR    [1],
			LF[1];
		 } *buf_zeile=(struct bufline*)bufadr;
   short za;

   zeilen++; /* Controllzeile muss mitberechnet werden */
   /* Alle Elemente umkopieren (Structur 0 ist keine Programmlinie :s. Protokoll)*/
   /* NB: Formatierung erfolgt spaeter */
   for (za=1;za<=zeilen;za++)
    {
      memcpy(cncprg[za-1].GM  ,buf_zeile[za].GM  ,3);
      memcpy(cncprg[za-1].XI  ,buf_zeile[za].XI  ,6);
      memcpy(cncprg[za-1].ZK  ,buf_zeile[za].ZK  ,6);
      memcpy(cncprg[za-1].FLKT,buf_zeile[za].FLKT,4);
      memcpy(cncprg[za-1].H   ,buf_zeile[za].H	 ,4);
    }
}

void Mem_EMCO(void)
{
   #define Serror  26
   #define Sescape 27
   int esc=27,weiter=0,zeile,bis_zeile;
   char buf[45],
	contr[32]={"    N` G`   X `    Z `  F`  H "};

   /* letzte Zeile suchen */
   for (zeile=max_line-1;zeile>0 && !strcmp(cncprg[zeile].GM,"    ");zeile--)
     bis_zeile=zeile;

   /* Raus wenn kein Programm im Speicher */
   if (bis_zeile==1) return;

   /* Ende wenn COM nicht vorhanden */
   if (!SYS_ExistCOM(prgconfig.com))
    {
      SYS_PushColor();
      _settextcolor(3);
      _setbkcolor(9);
      TW_Open_Window(25,10,49,3,TW_RAHMEN_S1,TW_TITEL_OBEN,5,"Erreur");
      _outtext(" Erreur:\n Interface sÇrie configurÇe n'existe pas!\n (Touche s.v.p.)");
      getch();
      TW_Close_Window();
      SYS_PopColor();
      return;
    }

   SYS_PushColor();
   _settextcolor(3);
   _setbkcolor(9);
   TW_Open_Window(25,10,45,7,TW_RAHMEN_S1,TW_TITEL_OBEN,5,"COM: -> EMCO");

   /* Maske */
   sprintf(buf,"Ligne :%d (env. 1 sec. par ligne)",bis_zeile-1);
   _outtext("Programm est transmis  par le port sÇrie versla maschine EMCO:\n");
   _outtext(buf);
   _outtext("\nLigne :ID");
   _settextposition(5,8);  _outtext(">");
   _settextposition(5,39); _outtext("<");

   /* Initialisierung der Schnittstelle */
   SER_Setzte_Parameter(prgconfig.com,DATABITS_7+STOPBIT_1+PARIT_EVEN+BAUD_300);

   /* Formatieren & Syntaxcheck */
   Format();
   weiter=1;
   if (Syntax()) {
		   _settextposition(6,12);
		   _outtext("Erreur dans le programme!");
		   _settextposition(7,6);
		   _outtext("Quand màme transmettre?      ");
		   weiter=SYS_JaNein(30,7,0,0);
		   _settextposition(6,12);
		   _outtext("                         ");
		 }

   if (weiter)
    {
      _settextposition(7,1);
      _outtext("      Frappez une touche pour commencer.    ");
      esc=getch();
      _settextposition(7,1);
      _outtext("           (Interruption avec ESC)         ");

      /* Startcode schreiben */
      SER_Schreibe_Zeichen(prgconfig.com,'%');
      SER_Schreibe_Zeichen(prgconfig.com,0xd); /* CR */
      SER_Schreibe_Zeichen(prgconfig.com,0xa); /* LF */

      /* Ueberschrift schreiben */
      _settextposition(5,9);
      esc=String_ComPort(0,contr,30);
      if (esc!=Serror)
	{
	  SER_Schreibe_Zeichen(prgconfig.com,0xd); /* CR */
	  SER_Schreibe_Zeichen(prgconfig.com,0xa); /* LF */
	}

      /* Programm */
      zeile=0;
      do {
	   /* Bildschirm -> Punkte */
	   _settextposition(5,9);
	   _outtext("                              ");
	   _settextposition(4,8);
	   sprintf(buf,"%03d   ",zeile);
	   _outtext(buf);
	   _settextposition(5,9);

	   /* Daten schreiben */
	   esc=String_ComPort(esc,"    ",3);
	   sprintf(buf,"%03d",zeile);
	   if (zeile<100) buf[0]=' ';
	   esc=String_ComPort(esc,buf,3);

	   esc=String_ComPort(esc,cncprg[zeile].GM,3);
	   esc=String_ComPort(esc," ",1);
	   esc=String_ComPort(esc,cncprg[zeile].XI,6);
	   esc=String_ComPort(esc,cncprg[zeile].ZK,6);
	   esc=String_ComPort(esc,cncprg[zeile].FLKT,4);
	   esc=String_ComPort(esc," ",1);
	   esc=String_ComPort(esc,cncprg[zeile].H,3);

	   if (esc!=Serror)
	    {
	      SER_Schreibe_Zeichen(prgconfig.com,0xd); /* CR */
	      SER_Schreibe_Zeichen(prgconfig.com,0xa); /* LF */
	    }
	   /* ESCape abfragen */
	   if (kbhit() && esc!=Serror) esc=getch();

	 }while (++zeile<bis_zeile && esc!=Sescape && esc!=Serror);
    } /* von Uebertragen wenn erleubt */

   /* Endcode auch im Fall von ESC senden */
   if (esc!=Serror)
    {
      SER_Schreibe_Zeichen(prgconfig.com,' ');
      SER_Schreibe_Zeichen(prgconfig.com,' ');
      SER_Schreibe_Zeichen(prgconfig.com,' ');
      SER_Schreibe_Zeichen(prgconfig.com,'M');
      /*SER_Schreibe_Zeichen(prgconfig.com,0xd);
      SER_Schreibe_Zeichen(prgconfig.com,0xa);*/
    }

   /* Alles beendet */
   _settextposition(7,1);
   if (esc==Sescape && esc!=Serror)
		_outtext("                Interruption!              ");
   if (esc!=Sescape && esc!=Serror)
		_outtext("            Transmission tÇrminÇe          ");
   if (esc!=Sescape && esc==Serror)
		_outtext("      Erreur pendant la transmission!      ");
   getch();
   SYS_PopColor();
   TW_Close_Window();
}

/* Gibt einen String (von 'counter' Zeichen) auf COM: aus */
/* return =27 wenn ESC */
int String_ComPort(int esc,char *st,char counter)
{
  int za,ret;

  if (esc==27) return esc;

  for (za=0;za<counter && esc!=27 && esc!=26;za++)
      {
	ret=SER_Schreibe_Zeichen(prgconfig.com,st[za]);
	if (!ret) { printf("\a"); esc=26; } /* 26=Errorcode / 27=ESCcode */
	    else {
		   if (kbhit()) esc=getch();  /* Nur Abfragen & Ausgeben wenn ! ESC */
		   _outtext(".");
		 }
      }
  return esc;
}


void No_Break(void)
{
  oldbreak=_dos_getvect(0x23);
  _dos_setvect(0x23,newbreak);
}

void  _interrupt _far newbreak(void)
{
/* rien */
}
