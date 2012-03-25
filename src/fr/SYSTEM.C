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


#include "system.h"
#include <conio.h>
#include <string.h>

/* Interne Variabeln */
static long  oldpaper[10];
static short oldpen[10];
static char  color_SP=-1;

/* FUNCTIONEN */

/**************************************************************************
Funktion erzeugt einen Farpointer aus dem angegebenen Segment und Offset.


void SYS_MakeFarPtr(unsigned sgm,unsigned ofs,void _far **farptr)
{
  _asm{
	push si
	mov ax,ofs
	mov si,farptr
	mov [si],ax
	inc si
	inc si
	mov ax,sgm
	mov [si],ax
	pop si
      }
}
*/

/**************************************************************************
Gibt das Statusbyte des Drucker zurÅck:
Bit 0:falsche Kanalnummer LPT1/2..
    3:I/O-Fehler (Ausschalten wÑhrend Druck)
    4:Drucker ON LINE
    5:kein Papier mehr
    6:Drucker antwortet
    7:Drucker wartet

Var "lpt": 0=LPT1 1=LPT2 ...
*/
char SYS_DruckerStatus(char lpt)
{
  union REGS inregs,outregs;

  inregs.h.ah=2;
  inregs.x.dx=lpt;
  int86(23,&inregs,&outregs);
  return outregs.h.ah;
}

/**************************************************************************
Initialisiert Drucker auf einer bestimmten Schnittstelle
Ret: Druckerstatus : s. ^
*/
char SYS_DruckerInit(char lpt)
{
  union REGS inregs,outregs;

  inregs.h.ah=1;
  inregs.x.dx=lpt;
  int86(23,&inregs,&outregs);
  return outregs.h.ah;
}

/**************************************************************************
Sendet ein Byte	zum Drucker Schnittstelle
Ret: Druckerstatus : s. ^
*/
char SYS_DruckerPrint(char lpt,char zeichen)
{
  union REGS inregs,outregs;

  inregs.h.ah=0;
  inregs.h.al=zeichen;
  inregs.x.dx=lpt;
  int86(23,&inregs,&outregs);
  return outregs.h.ah;
}

/**************************************************************************
Textcursor einschalten.
*/
void SYS_CursorOn(void)
{
  union REGS regs;

  regs.h.ah=1;
  if (SYS_ColorCard())
	    {
	      regs.h.ch=6;
	      regs.h.cl=7;
	    }
	else {
	      regs.h.ch=11;
	      regs.h.cl=14;
	     }

  int86(16,&regs,&regs);
}


/**************************************************************************
Textcursor ausschalten.
*/

void SYS_CursorOff(void)
{
  union REGS regs;

  regs.h.ah=1;
  regs.h.ch=30;
  regs.h.cl=29;
  int86(16,&regs,&regs);
}

char SYS_ColorCard(void)
{
  union REGS outregs,inregs;

  inregs.h.ah=0x0f;
  int86(0x10,&inregs,&outregs);
  if (outregs.h.al!=7)
     return 1;

  return 0;
}

void SYS_PushColor(void)
{
  color_SP++;
  oldpen[color_SP]  =_gettextcolor();
  oldpaper[color_SP]=_getbkcolor();
}

void SYS_PopColor(void)
{
  _settextcolor(oldpen[color_SP]);
  _setbkcolor(oldpaper[color_SP]);
  color_SP--;
}

void SYS_TextInvert(void)
{
   short pen=(char)_gettextcolor(),
	 paper=(char)_getbkcolor();

   _setbkcolor((long)pen);
   _settextcolor((short)paper);
}

char SYS_JaNein(char xp,char yp,char voreinst,char escret)
{
   char ret=voreinst,taste;

   do {
	if (ret)
	    {
	       _settextposition(yp,xp);
	       SYS_TextInvert();
	       _outtext(" Oui ");
	       SYS_TextInvert();
	       _outtext(" /  Non ");
	    }
	  else {
		 _settextposition(yp,xp);
		 _outtext(" Oui / ");
		 SYS_TextInvert();
		 _outtext(" Non ");
		 SYS_TextInvert();
	       }

	while (!kbhit());
	taste=(char)getch();
	if (taste==0) taste=(char)getch();

	switch (taste)
	{
	  case 77:ret=0; break;
	  case 75:ret=1; break;
	  case 27:ret=escret;
	}
      } while (taste!=13 && taste!=27);
  return ret;
}

char *SYS_DelEndSpaces(char *string)
{
  short x=(short)strlen(string)-1;
  while (string[x]==' ')
    string[x--]=0;
  return string;
}

char SYS_ExistCOM(char c)
{
  short x=c*2;
  char r;

  _asm {
	 push ds
	 mov ax,0x40
	 mov ds,ax

	 mov bx,x
	 mov al,[bx]
	 mov r,al

	 pop ds
       }

  return r;
}

char SYS_ExistLPT(char l)
{
  short x=8+l*2;
  char r;

  _asm {
	 push ds
	 mov ax,0x40
	 mov ds,ax

	 mov bx,x
	 mov al,[bx]
	 mov r,al

	 pop ds
       }

  return r;
}
