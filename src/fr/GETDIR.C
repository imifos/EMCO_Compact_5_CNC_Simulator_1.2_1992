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

#include "getdir.h"
#include "discerr.h"

/* VARIABELN & interne Prototypen */

void Schreibe_Linie(char,short);

       short		    DIR_eintraege;
static struct DIR_rec _far *DIR_top=NULL;

/* FUNCTIONEN */

char DIR_Wahl(char xp,char yp,char *suchstr, /*ret:*/char *name,char *pfad,char *laufw)
{
    struct DIR_rec dir;
    char taste=0,raus,za,
	 ret=1 /*=OK*/;
    int drive=_getdrive();
    short zeile1,yfen;

    /* Fenster */
    TW_Open_Window(xp,yp,37,10,TW_RAHMEN_D1,TW_TITEL_RECHTS,2," Dir");

    /* Dir holen */
    do
      {
	 raus=0;
	 DIR_Make(suchstr,/*_A_NORMAL |*/ _A_SUBDIR);

	 /* Ausgabe der ersten 10 */
	 _clearscreen(_GWINDOW);
	 for (zeile1=1;zeile1<11 && zeile1<=DIR_eintraege;zeile1++)
	   Schreibe_Linie((char)zeile1,zeile1);

	 /* Window-Schleife */
	 zeile1=1;
	 yfen=1;
	 do {
	      /* Invertbalken */
	      SYS_TextInvert();
	      Schreibe_Linie((char)yfen,zeile1+yfen-1);
	      SYS_TextInvert();

	      /* Auf Taste warten */
	      while (!kbhit());
	      taste=(char)getch();
	      if (taste==0) taste=(char)getch();

	      /* Balken loeschen */
	      Schreibe_Linie((char)yfen,zeile1+yfen-1);

	      /* Taste auswerten */
	      switch (taste)
	       {
		 case 72:/* rauf */
			 if (yfen==1 && zeile1!=1)
			   { /* scroll runter */
			     zeile1--;
			     _scrolltextwindow(_GSCROLLDOWN);
			     Schreibe_Linie(1,zeile1);
			   }
			   else /* balken hoch */
				if (yfen>1) yfen--;
			 break;
		 case 80:/* runter */
			if (yfen==10 && zeile1+9!=DIR_eintraege)
			  { /* scroll rauf */
			    zeile1++;
			    _scrolltextwindow(_GSCROLLUP);
			    Schreibe_Linie(10,zeile1+9);
			  }
			  else /* balken runter */
			       if (yfen<10 && zeile1+yfen-1<DIR_eintraege) yfen++;
	      } /*switch*/
	  } while (taste!=27 && taste!=13); /* Window-Schleife */

	 /*ESC*/
	 if (taste==27)
	     ret=0;

	 /* Enter-Bearbeitung */
	 if (taste==13)
	   {
	     DIR_ReadNr(zeile1+yfen-1,&dir);
	     if (!dir.dirident && !dir.driveident)
		{
		  /* Filename */
		  raus=1;
		  *laufw=(char)_getdrive();
		  getcwd(pfad,_MAX_PATH);
		  strcpy(name,dir.name+1);
		  /* Name zurechtschneiden */
		  for (za=(char)strlen(name);za>0;za--)
		     if (name[za]==' ') name[za]=0;
		}
	     if (dir.dirident)
		{
		  /* Directory */
		  for (za=(char)strlen(dir.name);za>0;za--)
		     if (dir.name[za]==' ') dir.name[za]=0;
		  chdir(dir.name+1);
		}
	     if (dir.driveident)
		_chdrive(dir.name[2]-'A'+1);
	     DE_ResetErrorMemory();
	   }

      }while (!raus && ret); /* von Dir-Schleife */


    /* Speicher loesen & Beenden*/
    TW_Close_Window();
    DIR_ReleaseAll();
    _chdrive(drive);

    return ret;
}

void Schreibe_Linie(char y,short nr)
{
   struct DIR_rec dir;
   char buf[70];

   DIR_ReadNr(nr,&dir);
   _settextposition(y,2);
   if (!dir.dirident && !dir.driveident)
      sprintf(buf," %s (%s) %7lu ",dir.name,dir.datum,dir.laenge);
      else
       sprintf(buf," %s                    ",dir.name,dir.datum,dir.laenge);

   _outtext(buf);
}


void DIR_Make	(char *name,unsigned attr)
{
   struct DIR_rec ds,_far *zgr;
   size_t laenge=sizeof(ds);
   struct find_t fileinfo;
   char buf[30],drive;
   int aktdr;

   DIR_ReleaseAll();

   /* LAUFWERKE EINLESEN */
   /* Wenn laufwerk anwaehlbar, dann existiert es */
   aktdr=_getdrive();
   for(drive=1;drive<27;drive++)
	if(!_chdrive(drive))
	  {
	    sprintf(ds.name,"[ %c:         ]",drive+'A'-1);
	    ds.driveident=1;
	    ds.dirident=0;
	    zgr=_fmalloc(laenge);
	    if (zgr!=NULL)
	      {
		 ds.vorher=DIR_top;
		 DIR_top=zgr;
		 _fmemmove(DIR_top,&ds,laenge);
		 DIR_eintraege++;
	      }
	  }
   _chdrive(aktdr);

   /* SUBDIRS EINLESEN EINLESEN */
   ds.driveident=0;
   ds.dirident=1;
   if (!_dos_findfirst("*.*",_A_SUBDIR,&fileinfo))
    do {
	 if(fileinfo.attrib & _A_SUBDIR)
	    {
	      while (strlen(fileinfo.name)<12)
		strcat(fileinfo.name," ");

	      strcpy(ds.name,"<");
	      strcat(ds.name,fileinfo.name);
	      strcat(ds.name,">");
	      zgr=_fmalloc(laenge);
	      if (zgr!=NULL && strcmp(ds.name,"<.           >"))
	       {
		  ds.vorher=DIR_top;
		  DIR_top=zgr;
		  _fmemmove(DIR_top,&ds,laenge);
		  DIR_eintraege++;
	       }
	    }
       } while(!_dos_findnext(&fileinfo));


   /* FILES EINLESEN */
   ds.driveident=0;
   ds.dirident=0;
   if (!_dos_findfirst(name,attr,&fileinfo))
    do {
	 if(!(fileinfo.attrib & _A_SUBDIR))
	    {  /* Eintrag */
	       while (strlen(fileinfo.name)<12)
		strcat(fileinfo.name," ");

	       strcpy(ds.name," ");
	       strcat(ds.name,fileinfo.name);
	       strcat(ds.name," ");

	       ds.laenge=fileinfo.size;
	       sprintf(buf,"%2.2d/%02.2d/%02.2d",fileinfo.wr_date & 0x1f,
		     (fileinfo.wr_date>>5) & 0x0f,(fileinfo.wr_date>>9)+80);

	       strcpy(ds.datum,buf);
	       zgr=_fmalloc(laenge);
	       if (zgr!=NULL)
		 {
		   ds.vorher=DIR_top;
		   DIR_top=zgr;
		   _fmemmove(DIR_top,&ds,laenge);
		   DIR_eintraege++;
		 }
	    }
	} while(!_dos_findnext(&fileinfo));

}


void DIR_ReleaseAll(void)
{
    struct DIR_rec speicher;
    size_t	   laenge;

    while (DIR_eintraege>0)
	{
	  laenge=sizeof(speicher);
	  _fmemmove((void _far*)&speicher,DIR_top,laenge);

	  _ffree(DIR_top);
	  DIR_top=speicher.vorher;
	  DIR_eintraege--;
	}
}


void DIR_ReadNr (short num,struct DIR_rec *dest)
{
    struct DIR_rec _far *zeiger=DIR_top,speicher;
    register short za,laenge,co;

    if (num<=DIR_eintraege && num>0)
       {
	 laenge=sizeof(speicher);
	 _fmemmove((void _far*)&speicher,zeiger,laenge);
	 co=DIR_eintraege-num;

	 for (za=1;za<=co;za++)
	    {
	       zeiger=speicher.vorher;
	       _fmemmove((void _far*)&speicher,zeiger,laenge);
	    }
	 *dest=speicher;
       }
}



/********** Vorlaufig nicht benutzt *************/
/* void DIR_Sort	   (void);
static void Swap	   (unsigned,unsigned);
y
void Swap (unsigned num1,unsigned num2)
{
    struct DIR_rec zw,speicher1,speicher2,_far *zgr1,_far *zgr2;
    register short za,laenge,co;
    void _far *zw1,_far *zw2;

    * Suche Adr Element 1 *
    laenge=sizeof(speicher1);
    _fmemmove((void _far*)&speicher1,DIR_top,laenge);
    co=DIR_eintraege-num1;
    for (za=1;za<=co;za++)
      {
	zgr1=speicher1.vorher;
	_fmemmove((void _far*)&speicher1,zgr1,laenge);
      }

    * Suche Adr Element 2 *
    _fmemmove((void _far*)&speicher2,DIR_top,laenge);
    co=DIR_eintraege-num2;
    for (za=1;za<=co;za++)
      {
	zgr2=speicher2.vorher;
	_fmemmove((void _far*)&speicher2,zgr2,laenge);
      }
   zw1=speicher1.vorher;
   zw2=speicher2.vorher;
   zw =speicher1;
   speicher1=speicher2;
   speicher2=zw;
   speicher1.vorher=zw1;
   speicher2.vorher=zw2;
   _fmemmove(zgr1,(void _far*)&speicher1,laenge);
   _fmemmove(zgr2,(void _far*)&speicher2,laenge);
}

void DIR_Sort(void)
{
   short x,y;
   struct DIR_rec elm1,elm2;

   for (x=1;x<DIR_eintraege;x++)
     for (y=2;y<DIR_eintraege;y++)
	{
	  DIR_ReadNr(y-1,&elm1);
	  DIR_ReadNr(y,&elm2);
	  if (strcmp(elm1.name,elm2.name)>0)
	     Swap(y-1,y);
	}
}
  */
