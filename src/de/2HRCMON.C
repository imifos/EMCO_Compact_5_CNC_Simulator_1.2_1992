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



#include "2hrcmon.h"

char VIDEO_sec =0,
     VIDEO_prim=0,
     MON2_vorhanden=0;

/* interner Prototyp */
char far* M2XAdr(char,char);

/* interne VARIABELN */
char xpos=1,ypos=1,    /* aktuelle Koordinaten: Begin bei 1,1 */
     attribut=7;       /* aktuelles Textattribut */

char _far *basis=(char _far*)0xB0000000;

/* FUNCTIONEN */
void MON2_clrscr(char x,char y,char br,char ho)
{
   char xp,yp;
   for (yp=y;yp<=ho;yp++)
    for (xp=x;xp<=br;xp++)
       *M2XAdr(xp,yp)=0;
}

void MON2_settextposition(char x,char y)
{
  xpos=x;
  ypos=y;
}

void MON2_outchar(char c)
{
   char _far *adr=M2XAdr(xpos,ypos);

   *adr=c;
   *(adr+1)=attribut;
   xpos++;
   if (xpos==81) { xpos=1; ypos++; }
}

void MON2_outshort(char *form,short var)
{
   char buf[50];

   sprintf(buf,form,var);
   MON2_outtext(buf);
}

void MON2_outtext(char _far *str)
{
   char _far *adr=M2XAdr(xpos,ypos);
   short za,txtza;

   for (za=0,txtza=0;str[txtza]!=0;txtza++)
    {
      *(adr+za++)=str[txtza];
      *(adr+za++)=attribut;
      xpos++;
      if (xpos==81) { xpos=1; ypos++; }
    }
}

void MON2_setattribut(char att)
{
  attribut=att;
}

char far* M2XAdr(char x,char y)
{
   char far *adr;

   x--;
   y--;
   adr=basis+x*2+y*160;
   return adr;
}

/* DIese funktion ist noch NICHT funktionsfaehig PC-INTERN 3 1191 */
char MON2_Aktiviere_Adapter(char adp)
{
  union REGS regs;

  if (!MON2_vorhanden || VIDEO_sec==0) return 0;
   else {
	  regs.h.ah=0x1a;				       /* Funktion */
	  regs.h.al=1;					       /* UnterFunktion */
	  regs.h.bl=(adp==MON2_SECOND) ? VIDEO_sec:VIDEO_prim; /* Aktive */
	  regs.h.bh=(adp==MON2_SECOND) ? VIDEO_prim:VIDEO_sec; /* Inaktive */
	  int86(0x10,&regs,&regs);
	 }

  return MON2_vorhanden;
}

char MON2_Init(void)
/* primaeren & secundaeren Videoadapter ermitteln & Systemvar setzten */
{
  union REGS regs;

  regs.h.ah=0x1a;   /* Funktion */
  regs.h.al=0;	    /* UnterFunktion */
  int86(0x10,&regs,&regs);

  MON2_vorhanden=MON2_ERR; /* Kein VGA-BIOS vorhanden */
  if (regs.h.al==0x1A)
	 {
	   VIDEO_sec=regs.h.bh;    /* 0 wenn nicht vorhanden */
	   VIDEO_prim=regs.h.bl;
	   if (VIDEO_sec==0x01)    /* 1=Code fuer MDA-Karte */
	       MON2_vorhanden=MON2_OK;
	 }

  return MON2_vorhanden;
}
