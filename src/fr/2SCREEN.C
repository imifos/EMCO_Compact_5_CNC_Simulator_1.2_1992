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
#include "cncsim.h"
#include "simul.h"
#include "2screen.h"

extern struct cncprgstr cncprg[max_line];
extern char st_durchmesser[6];
extern char st_laenge[6];
extern char st_innendm[6];

void EineZeile(char yp,short zeile)
{
   char buf[30];

   if (zeile<0 || zeile>=max_line)
    {
      MON2_settextposition(1,yp);
      MON2_outtext("                                        ");
      MON2_outtext("                                        ");
    }
    else {
	   /* Nummer */
	   MON2_settextposition(1,yp);
	   sprintf(buf,"%03d",zeile);
	   MON2_outtext(buf);

	   /* G/M */
	   MON2_settextposition(5,yp);
	   MON2_outtext(cncprg[zeile].GM);
	   /* X/I */
	   MON2_settextposition(11,yp);
	   MON2_outtext(cncprg[zeile].XI);
	   /* Z/K */
	   MON2_settextposition(20,yp);
	   MON2_outtext(cncprg[zeile].ZK);
	   /* F/L/K/T */
	   MON2_settextposition(29,yp);
	   MON2_outtext(cncprg[zeile].FLKT);
	   /* H */
	   MON2_settextposition(36,yp);
	   MON2_outtext(cncprg[zeile].H);
	   /* BEM */
	   MON2_settextposition(41,yp);
	   MON2_outtext(cncprg[zeile].BEM);
	 }
}

void Init2SCR(void)
{
   char buf_l[20],buf_d[20],buf_id[20],za;

   if (!MON2_vorhanden) return;

   sprintf(buf_l ,"%6.2f",atof(st_laenge)/100.0);
   sprintf(buf_id,"%6.2f",atof(st_innendm)/100.0);
   sprintf(buf_d ,"%6.2f",atof(st_durchmesser)/100.0);

   MON2_clrscr(1,1,80,25);
   MON2_settextposition(1,1);
   MON2_outtext("PiŠce - Longueur: ");
   MON2_outtext(buf_l);
   MON2_outtext("mm ³ DiamŠtre   :");
   MON2_outtext(buf_d);
   MON2_outtext("mm ³ DiamŠtre inter. :");
   MON2_outtext(buf_id);
   MON2_outtext("mm");

   MON2_settextposition(1,2);
   MON2_outtext("Coordonn‚es:   X:              Y:              Outil : T01");
   MON2_settextposition(1,3);
   MON2_outtext("ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");

   MON2_settextposition(1,10);
   MON2_outtext("ÄÄÄÂÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");
   MON2_settextposition(1,11);
   MON2_outtext(" N ³G/M ³  X/I   ³  Z/K   ³F/L/K/T³ H  ³ Commentaire");
   MON2_settextposition(1,12);
   MON2_outtext("   ³    ³        ³        ³       ³    ³");
   MON2_settextposition(1,13);
   MON2_outtext("ÄÄÄÁÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");

   MON2_settextposition(1,23);
   MON2_outtext("ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ");
   MON2_settextposition(1,24);
   MON2_outtext("Etat   :");
   MON2_settextposition(1,25);
   MON2_outtext("Message:");

   EineZeile(12,0);
   for (za=1;za<10;za++)
    EineZeile((char)(13+za),(short)za);
}

void Zeile2SCR(short zeile)
{
   short za;
   char y;

   if (!MON2_vorhanden) return;

   for (za=zeile-6,y=4;y<10;za++,y++)
    EineZeile(y,za);
   EineZeile(12,zeile);
   for (za=zeile+1,y=14;y<23;za++,y++)
    EineZeile(y,za);
}

void Koord2SCR(short x,short z)
{
  char buf_x[35],buf_z[35];

  if (!MON2_vorhanden) return;

  sprintf(buf_x,"%+06.2fmm ",(float)(x*2)/100.0F);
  sprintf(buf_z,"%+06.2fmm ",(float)z/100.0F);

  MON2_settextposition(18,2);
  MON2_outtext(buf_x);
  MON2_settextposition(34,2);
  MON2_outtext(buf_z);
}

void Tool2SCR(char *tool)
{
  if (!MON2_vorhanden) return;

  MON2_settextposition(56,2);
  MON2_outtext(tool);
  MON2_outtext("     ");
}

void Status2SCR(char *s)
{
  if (!MON2_vorhanden) return;
  MON2_settextposition(9,24);
  MON2_outtext("                                  ");
  MON2_settextposition(9,24);
  MON2_outtext(s);
}

void Meldung2SCR(char *s)
{
  if (!MON2_vorhanden) return;
  MON2_settextposition(9,25);
  MON2_outtext("                                                                ");
  MON2_settextposition(9,25);
  MON2_outtext(s);
}
